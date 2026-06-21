import test from 'ava';
import zlib from 'zlib';
import crypto from 'crypto';
import zlibSampleInit from '../src/index.js';


await test("inflate", async t => {
  const myZlib = await zlibSampleInit();
  let source;
  let deflated;

  for (let i = 0; i < 10; i++) {
    source = crypto.randomBytes(1);
    deflated = zlib.deflateRawSync(source, {});
    t.true(source.equals(myZlib.inflate(deflated)));

    source = crypto.randomBytes(1024);
    deflated = zlib.deflateRawSync(source, {});
    t.true(source.equals(myZlib.inflate(deflated)));

    source = crypto.randomBytes(1024 * 1024);
    deflated = zlib.deflateRawSync(source, {});
    t.true(source.equals(myZlib.inflate(deflated)));
  }
});

test("deflate", async t => {
  const myZlib = await zlibSampleInit();
  let source;
  let deflated;

  for (let i = 0; i < 10; i++) {
    source = crypto.randomBytes(1);
    deflated = myZlib.deflate(source);
    t.true(zlib.inflateRawSync(deflated).equals(source));

    source = crypto.randomBytes(1024);
    deflated = myZlib.deflate(source);
    t.true(zlib.inflateRawSync(deflated).equals(source));

    source = crypto.randomBytes(1024 * 1024);
    deflated = myZlib.deflate(source);
    t.true(zlib.inflateRawSync(deflated).equals(source));
  }
});
