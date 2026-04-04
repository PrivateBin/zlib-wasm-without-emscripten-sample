import Benchmark from 'benchmark';
import fs from 'fs';
import pako from 'pako';

import zlibSampleInit from '../src/index.js';
import zlib from 'zlib';

(async () => {
  const myZlib = await zlibSampleInit();
  const chunkSize = 32 * 1024;
  const level = 6;
  const source = fs.readFileSync("benchmark/lorem_1mb.txt");

  // warmup
  myZlib.deflate(source);
  zlib.deflateRawSync(source, { chunkSize, level });
  pako.deflateRaw(source, { chunkSize, level });

  // run
  console.log(`## lorem_1mb.txt size: ${source.length}`);

  const suiteDef = new Benchmark.Suite("deflate");

  suiteDef
    .add('wasm', {
      fn: () => myZlib.deflate(source),
    })
    .add('pako', {
      fn: () => pako.deflateRaw(source, { chunkSize, level }),
    })
    .add('native', {
      fn: () => zlib.deflateRawSync(source, { chunkSize, level }),
    })
    .on('cycle', (event) => {
      console.log(String(event.target));
    })
    .on('complete', () => {
      console.log('Deflate: Fastest is ' + suiteDef.filter('fastest').map('name'));
    })
    .run();

  // warmup
  const deflated = zlib.deflateRawSync(source, { level: 6 });
  myZlib.inflate(deflated);
  pako.inflateRaw(deflated, { chunkSize });
  zlib.inflateRawSync(deflated, { chunkSize });

  console.log(`## deflated lorem_1mb.txt size: ${deflated.length}`);

  const suiteInf = new Benchmark.Suite("inflate");

  suiteInf
    .add('wasm', {
      fn: () => myZlib.inflate(deflated),
    })
    .add('pako', {
      fn: () => pako.inflateRaw(deflated, { chunkSize }),
    })
    .add('native', {
      fn: () => zlib.inflateRawSync(deflated, { chunkSize }),
    })
    .on('cycle', (event) => {
      console.log(String(event.target));
    })
    .on('complete', () => {
      console.log('Inflate: Fastest is ' + suiteInf.filter('fastest').map('name'));
    })
    .run();

})();
