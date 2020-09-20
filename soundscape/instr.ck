TubeBell s[36] => dac;
for( 0 => int i; i < 36; i++) {
  s[i].freq(i/36.0 * 1000.0 + 440.);
}
OscRecv orecv;
4561 => orecv.port;
orecv.listen();

function void note_on() {
  orecv.event("/lidar/on,i,f") @=> OscEvent note_on;
  while(true) {
    note_on => now;
    while(note_on.nextMsg() != 0) {
      note_on.getInt() => int index;
      note_on.getFloat() => float velocity;

      index * 1000.0 / 36.0 + 440.0 => s[index].freq;
      velocity => s[index].noteOn;
    }
  }
}

function void note_off() {
  orecv.event("/lidar/off,i") @=> OscEvent note_off;
  while(true) {
    note_off => now;
    while(note_off.nextMsg() != 0) {
      note_off.getInt() => int index;
      s[index].noteOff(0.);
    }
  }
}

function void note_move() {
  orecv.event("/lidar/move,i,f") @=> OscEvent note_move;
  while(true) {
    note_move => now;
    while(note_move.nextMsg() != 0) {
      note_move.getInt() => int index;
      note_move.getFloat() => float velocity;

      index * 1000.0 / 36.0 + 440.0 => s[index].freq;
      // velocity => s[index].noteOn;
    }
  }
}


spork ~ note_on();
spork ~ note_off();

while(true)
  1::second => now;
