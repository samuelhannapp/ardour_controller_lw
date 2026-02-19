# Ardour OSC Controller 

This is the way how a DAW is supposed to be controlled.
Main thing's are, navigation via spill feature and customized plugin control, meaning the user can route the plugin parameters 
customized however he want's.

### Motivation

Mackie Controller are very limited, because of two reason's.
Fist is navigation, you can only navigate through all strips via banking, there is no feature like, folder categories, 
buses or vcas where you could group strips together and spill them out on the control surface.
Ardour Osc has the spill feature, so the first thing is, you can either via vca or bus select a category and than spill
it on the surface.

Second reason is, for most DAW's plugin control is limited to 8 parameters.
Beause of this it's pretty useless, because lot's of channelstrips have significantly more than 8 plugin paramters.
And also the knobs of the mackie control are teribble.

This project  attempts to explore what is possible with Ardour osc.
There are three types of controllers.

One is a base controller. That can be a Mackie control. It has extended features as described above, and also 
all of the standart features work.

The second one is a General Purpose controller.
This is a plugin controller with custom user defined paramter routing if wished.

The third version is a Special Purpose controller.
That means it's only controlling one specific plugin, for example an SSL EQ, and 
every time a strip is selected with the respective plugin, no mether in wich place in 
the strip, it will be represented in the Special purpose controller and can be controlled 
from there.

There are gui versions of these things for trying.
Any DAW Controller with the Mackie Protocol is compatible.
And I'll make some attempts to build some hardware versions for testing.
The plane is to use endless potentiometers or quadrature encoders or 
potentiometers with stepper motor's or three phase motors.

When I started this project I just wanted to write custom software with Mackie protocol to 
control plugins, after a while I realised this is so limited that its pretty much useless.
Than I found the Osc protocol and Ardour and was very exited about the capabilities and 
decided to test some thing's out, and it looks all very promising to me.

There is a version wich works on Win/Linux/Mac and also on STM32 Microcontroller.

There are some flaws in the software and the OSC protocol.
I'll try later to addres them...

Another nice thing is, there is a display for the base controller, wich can be an 
alternative to the mackie LCD display and looks much more nice!!!!!!!!

There will be manuals for all things to test and to build yourself.

And maybe one day hardware could be produced and sold.

I think in general the Osc protocol should be the way to go to control a DAW.
It would be a very nice thing if more DAW's would implement it.
Ardour will be a rolemodel for that.

### License

This project is licensed under the MIT license.

---

# Setup


## Supported platforms

| Platform | Supported |
| -------- | --------- |
| Windows  | ✅         |
| MacOS    | ✅         |
| Linux    | ✅         |         |
| STM32    | ✅         |         |
| Rasperry | ✅         |         |

## Goal


| Feature                                         				| Supported |
| ----------------------------------------------- 				| --------- |
| Spill Feature                           						| ✅         |
| Custom Plugin routing                         				| ✅         |
| Mackie Control Protocol Compatibility 						| ✅         |
| Base Controller Gui Version                       			| ✅         |
| General Purpose Controller Gui Version            			| ✅         |
| Special Purpose Controller Gui Version            			| ✅         |
| Base Controller Hardware Version (Mackie)         			| ✅         |
| General Purpose Controller Hardware Version potentiometer		| ❌         |
| Special Purpose Controller Hardware Version potentiometer     | ❌         |
| General Purpose Controller Hardware Version stepper           | ❌         |
| Special Purpose Controller Hardware Version stepper           | ❌         |
| General Purpose Controller Hardware Version quadrature        | ❌         |
| Special Purpose Controller Hardware Version quadrature        | ❌         |
| Base Controller Display                               		| ✅         |
