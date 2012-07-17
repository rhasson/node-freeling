/*
*  Freeling binding interface module
*/

var binding = process.binding('freeling');
var util = require('util');
var stream = require('stream');

binding.TOKENIZER_CONFIG_PATH = "/home/roy/freeling/free3/share/tokenizer.dat";
binding.SPLITTER_CONFIG_PATH = "/home/roy/freeling/free3/share/splitter.dat";
binding.FLUSH = false;

exports.config = {
	TOKENIZER_CONFIG_PATH: binding.TOKENIZER_CONFIG_PATH,
	SPLITTER_CONFIG_PATH: binding.SPLITTER_CONFIG_PATH,
	FLUSH: binding.FLUSH
};
Object.keys(exports.config).forEach(function(k) {
  exports.config[exports.config[k]] = k;
});

var mode = {
	TOK: 1,
	SPL: 2
};

//interface for creating NLP tool objects
//o.path -> full path and file name for the tool's configuration file
function createTokenizer(o) {
	return new Tokenizer(o);
}
function createSplitter(o) {
	return new Splitter(o);
}

//constructor functions for each NLP tool
function Tokenizer(o) {
  if (!(this instanceof Tokenizer)) return new Tokenizer(o);
  Freelib.call(this, mode.TOK, o);
}
function Splitter(o) {
  if (!(this instanceof Splitter)) return new Splitter(o);
  Freelib.call(this, mode.SPL, o);
}

/**********************************************************************/
//convenience methods
exports.tokenize = function(buf, cb) {
	var o = {path: binding.TOKENIZER_CONFIG_PATH};
	flibBuffer(new Tokenizer(o), buf, cb);
}
exports.split = function(buf, cb) {
	var o = {path: binding.SPLITTER_CONFIG_PATH};
	flibBuffer(new Splitter(), buf, cb);
}
/**********************************************************************/

//Wrapper object to help manage the convenient methods interaction with Freeling binding
function flibBuffer(engine, buffer, callback) {
	var buffers [];
	var nreads = 0;

	function onError(err) {
		engine.removeListener('end', onEnd);
		engine.removeListener('error', onError);
		callback(err);
	}

	function onData(chunk) {
		buffers.push(chunk);
		nread += chunk.length;
	}

	function onEnd() {
		var buf;
		switch (buffers.length) {
			case 0:
				buf = new Buffer(0);
				break;
			case 1:
				buf = buffers[0];
				break;
			default:
				buf = new Buffer(nread);
				var n = 0;
				buffers.forEach(function(c) {
					var l = c.length;
					c.copy(buf, n, 0, l);
					n += 1;
				});
				break;
		}
		callback(null, buf);
	}
	engine.on('error', onError);
	engine.on('data', onData);
	engine.on('end', onEnd);

	engine.write(buffer);
	engin.end();
}

/*
* Base class all JS NLP interfaces inherits from
* Manages communication with the Freeling binding, queues, etc.
*/
function Freelib(mode, opts) {
	var self = this;

	self._opts = opts;
	self._queue = [];
	self._processing = false;
	self._ended = false;
	self._hadError = false;
	self.readable = true;
	self.writeable = true;
	self._flush = binding.FLUSH;
	self._chunkSize = 8192;

	switch(mode) {
		case 1: 
			self._binding = new binding.Tokenizer(opts.path);
			break;
		case 2: 
			self._binding = new binding.Splitter(opts.path);
			break;
	};

	self._binding.onerror = function(message, errno) {
		self._binding = null;
		self._hadError = true;
		self._queue.length = 0;
		self._processing = false;

		var error = new Error(message);
		error.errno = errno;
		self.emit('error', error);
	}

	self._buffer = new Buffer(self._chunkSize);
	self._offset = 0;
}

util.inherits(Freelib, stream.Stream);

Freelib.prototype._process = function() {
	var self = this;

	if (self._hadError) return;

	if (self._processing || self._paused) return;

	if (self.queue.length === 0) {
		if (self._needDrain) {
			self._needDrain = false;
			this.emit('drain');
		}
		return;
	}

	//queue = array of [chunk, cb]
	var req = self._queue.shift();
	var cb = req.pop();
	var chunk = req.pop();

	if (self._ending && self._queue.length === 0) self._flush = true;

	var availInBefore = chunk && chunk.length;
	var availOutBefore = self._chunkSize - self._offset;
	var inOff = 0;
	var req = self._binding.write(self._flush,  
									chunk, //input buffer
									inOff,  //input offset
									availInBefore,  //input buffer length
									self._buffer,  //output buffer
									self._offset,  //output offset
									availOutBefore);  //output buffer length
	req.buffer = chunk;
	req.callback = callback;
	this._procesing = req;

	function callback(availInAfter, availOutAfter, buffer) {
		if (self._hadError) return;

		var have availOutBefore - availOutAfter;
		assert(have >= 0, 'have should not go down');
		if (have > 0) {
			var out = self._buffer.slice(self._offset, self._offset + have);
			self._offset += have;
			self.emit('data', out);
		}

		if (availOutAfter === 0 || self._offset >= self._chunkSize) {
			availOutBefore = self._chunkSize;
			self._offset = 0;
			self.buffer = new Buffer(self._chunkSize);
		}

		if (availOutAfter === 0) {
			inOff += (availInBefore - availInAfter);
			availInBefore = availInAfter;

			var newReq = self._binding.write(self._flush,
												chunk,
												inOff,
												availInBefore,
												self._buffer,
												self._offset,
												self._chunkSize);
			newReq.callback = callback;
			newReq.buffer = chunk;
			self._processing = newReq;
			return;
		}

		self._processing = false;
		if (cb) cb();
		self._process();
	}
}

Freelib.prototype.write = function write(chunk, cb) {
	if (this._hadError) return true;

	if (this._ended) {
			return this.emit('error', new Error('Cannot write after end'));
	}
	if (arguments.length === 1 && typeof chunk === 'function') {
		cb = chunk;
		chunk = null;
	}

	if (!chunk) chunk = null;
	else if (typeof chunk === 'string') chunk = new Buffer(chunk);
	else if (!Buffer.isBuffer(chunk)) return this.emit('error', new Error('First argument needs to be a string or buffer'));

	var empty = this._queue.length === 0;

	this._queue.push([chunk, cb]);
	this._process();
	if (!empty) this._needDrain = true;
	return empty;
}

Freelib.prototype.flush = function flush(cb) {
	this._flush = true;
	return this.write(cb);
}

Freelib.prototype.end = function end(chunk, cb) {
	if (this._hadError) return true;

	var self = this;
	var ret = this.write(chunk, function(){
		self.emit('end');
		if (cb) cb();
	});
	this._ended = true;
	return ret;
}

Freelib.prototype.pause = function() {

}

Freelib.prototype.resume = function() {

}

Freelib.prototype.destroy = function() {

}

util.inherits(Tokenizer, Freelib);
util.inherits(Splitter, Freelib);