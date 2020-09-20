SinOsc s => dac;
s.freq(440);
s.op(0);
OscRecv orecv;
4560 => orecv.port;
orecv.listen();

function void freq_shred() {
  orecv.event("/2/vol1,f") @=> OscEvent freq;
  while(true) {
    freq => now;
    while(freq.nextMsg() != 0) {
      freq.getFloat() * 1000.0 + 440.0 => float value;
      <<< value >>>;
      s.freq(value);
    }
  }
}

function void trigger_shred() {
  orecv.event("/1/push1,f") @=> OscEvent trigger;

  while( true ) {
    trigger => now;

    while(trigger.nextMsg() != 0) {
      trigger.getFloat() => float value;
      if (value > 0.5) {
        <<< "440 on" >>>;
        s.op(1);
      } else {
        <<< "440 off" >>>;
        s.op(0);
      }
    }
  }
}

spork ~ trigger_shred();
spork ~ freq_shred();

while(true)
  1::second => now;
