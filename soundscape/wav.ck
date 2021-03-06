[ "sounds/clip00.wav",
"sounds/clip01.wav",
"sounds/clip02.wav",
"sounds/clip03.wav",
"sounds/clip04.wav",
"sounds/clip05.wav",
"sounds/clip06.wav",
"sounds/clip07.wav",
"sounds/clip08.wav",
"sounds/clip09.wav",
"sounds/clip10.wav",
"sounds/clip11.wav",
"sounds/clip12.wav",
"sounds/clip13.wav",
"sounds/clip14.wav",
"sounds/clip15.wav",
"sounds/clip16.wav",
"sounds/clip17.wav",
"sounds/clip18.wav",
"sounds/clip19.wav",
"sounds/clip20.wav",
"sounds/clip21.wav"
 ] @=> string files[];
files.cap() => int fileCount;

SndBuf s[36];
for( 0 => int i; i < fileCount; i++) {
  me.dir() + "/" + files[i] => s[i].read;
  s[i].samples() => s[i].pos;
  if (i%2 == 0) {
    s[i] => dac.left;
  } else {
    s[i] => dac.right;
  }
}

SinOsc beep => dac;
beep.freq(440);
0.5::second => now;
beep.op(0);

OscRecv orecv;
4561 => orecv.port;
orecv.listen();

function void note_on() {
  orecv.event("/lidar/on,i,f") @=> OscEvent note_on;
  while(true) {
    note_on => now;
    while(note_on.nextMsg() != 0) {
      note_on.getInt() % fileCount => int index;
      note_on.getFloat() => float velocity;

      velocity + 0.15 => s[index].rate;
      if (s[index].pos() >= s[index].samples()) {
        0 => s[index].pos;
      }
    }
  }
}

function void note_off() {
  orecv.event("/lidar/off,i") @=> OscEvent note_off;
  while(true) {
    note_off => now;
    while(note_off.nextMsg() != 0) {
      note_off.getInt() => int index;
      // s[index].noteOff(0.);
    }
  }
}

function void note_move() {
  orecv.event("/lidar/move,i,f") @=> OscEvent note_move;
  while(true) {
    note_move => now;
    while(note_move.nextMsg() != 0) {
      note_move.getInt() % fileCount => int index;
      note_move.getFloat() => float velocity;

      velocity + 0.15 => s[index].rate;
      if (s[index].pos() >= s[index].samples()) {
        0 => s[index].pos;
      }
    }
  }
}


spork ~ note_on();
// spork ~ note_off();
spork ~ note_move();

while(true)
  1::second => now;
