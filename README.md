# Lidar Scanner

An openframeworks sketch that streams information from a RPLidar A1 out to OSC.

It bins the 360 degrees of the lidar output into 36 10deg segments. The average distance of all degrees in a segment
are averaged with the previous average value in the last frame.

The Osc messages are filtered based on "significant" changes of a segment.  Significant is dictated by an absolute
difference between the current frame and the previous frame's distance.

This sketch does require my fork of [ofxRPlidar](https://github.com/RussTheAerialist/ofxRPlidar) which you'll want to
install in your `addons` directory.

## OSC Output Structure (output)

## Argument definitions

* `degreebin` - **int** Degree Bin is the bin which correlates to the angle of the reading. In the default case, it's a 10 degree
  segment
* `velocity` - **float** The value of the segment which we call velocity here to map to the idea of MIDI. It's the rolling
  average of the previous frames value for this segment, plus all new data. The range is 0.0 - 1.0 where 0.0 is close to
	infinity in distance of detection (e.g. no obstacle) and where 1.0 means closest detection.

## Messages

* `/lidar/on [degreebin] [velocity]` - A segment activation detected with a certain velocitory
* `/lidar/off [degreebin]` - A segment deactivation based on the velocity falling below a certain value
* `/lidar/move [degreebin] [velocity]` - A segment's distance has change enough that it excited a change signal

## OSC Tuneables Structure (input)

The Tuneable features of sketch are controllable via OSC messages. It's designed to be used by
[TouchOSC](https://hexler.net/products/touchosc) using page 2 of the LiveControl layout. See the below image for
information on what controls in the layout make to functionality.

* **bottom & top** - Top and Bottom are the lower bounds and upper bounds of the distance. Since distance is modeled
  as millimeters, this maps to max and min distances. Anything below bottom will clamp to bottom, and anything above
	top will clamp to top.
* **change threshold** - If the absolute difference between the last frame and the current frame is greater than the
	threshold then an OSC message will be triggered based on the current velocity.
* **save** - Writes the parameters to `settings.xml` under the `data` directory. This file is automatically loaded
  on startup.
* **debug** - Debug toggles between the current velocity/distance average of each segment, and the change activation
  visualization.

*TODO! Add Image*

## TODO

* [ ] Create TouchOSC layout
* [ ] Create a load settings button to reset without having to restart the app
* [ ] Add the ability to pause and resume OSC output
* [ ] Add ability to set connection port and host that can be adjusted at runtime
* [ ] Add ability to adjust the average weighting