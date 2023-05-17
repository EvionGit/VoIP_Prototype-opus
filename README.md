# VoIP Prototype (OPUS)
This PET-project represents a simple prototype of a VoIP application. Created using simple multimedia libraries ([SFML](https://www.sfml-dev.org/index.php) and [ImGui](https://github.com/MingNian/ImGui)).
Based on the [OPUS open-source audio codec](https://opus-codec.org/).
___

## Project structure
### The project consists of 4 modules:
+ [Data transmission](https://github.com/EvionGit/VoIP_Prototype-opus/edit/main/README.md#module-1-data-transmission)
+ [Audio data capture and compression](https://github.com/EvionGit/VoIP_Prototype-opus#module-2-audio-capture-and-compression)
+ [Data decompression and playback](https://github.com/EvionGit/VoIP_Prototype-opus#module-3-audio-decompression-and-playback)
+ [Controller](https://github.com/EvionGit/VoIP_Prototype-opus#module-4-controller)

___
## The diagram below shows all classes and their relationships
![](https://github.com/EvionGit/VoIP_Prototype-opus/blob/main/readme-src/VoIP_UML_CLASSES.png)
___

### Module 1. *Data Transmission*
Data is transmitted via sockets with raw UDP protocol. *(the goal of the project was to implement network data transmission without the use of additional protocols, such as RTP and SIP. Packet control is carried out using proprietary methods implemented with a JitterBuffer)*

<ins>***Classes for working with sockets are implemented using a wrapper over WinSock, defined in the [namespace :: wsock](https://github.com/EvionGit/VoIP_Prototype-opus/tree/main/include/wsock)***</ins>

At startup, an another thread with the <ins>VoIP::multiplex()</ins> method is started in the [VoIP controller](https://github.com/EvionGit/VoIP_Prototype-opus/blob/main/include/VoIP.h). Which listens to the socket and, depending on the type of packet, manages the application.

<ins>***Package structures and methods of their processing are defined in the [namespace :: pack](https://github.com/EvionGit/VoIP_Prototype-opus/tree/main/include/pack)***</ins>
<br><br><br>
___

### Module 2. *Audio capture and compression*
Audio signal capture is processed in the <ins>stream::AudioStreamIn</ins> class, which implements the abstract SFML library class <ins>sf::SoundRecorder</ins>.
The recorded data is transmitted in chunks of 20ms to the Opus encoder, which, depending on the program settings, encodes the data and redirects it to the network using the <ins>stream::NetStreamAudioOut</ins>, pre-providing headers.

<ins>***Write streams are defined in the [namespace :: stream](https://github.com/EvionGit/VoIP_Prototype-opus/tree/main/include/stream), and the encoder in the [namespace :: ops](https://github.com/EvionGit/VoIP_Prototype-opus/tree/main/include/ops)***</ins>
<br><br><br>
___

### Module 3. *Audio decompression and playback*
The encoded data enters the [jitter buffer](https://github.com/EvionGit/VoIP_Prototype-opus/blob/main/include/jbuf/jitter_buffer.h) queue, where it is buffered and ordered. Further, the data at the request of the audio player implemented in <ins>stream::AudioStreamOut</ins> is decoded using <ins>ops::Decoder</ops> and submitted for playback.

<ins>***Output streams, decoder and jitter buffer are implemented in [namespace :: stream](https://github.com/EvionGit/VoIP_Prototype-opus/tree/main/include/stream),[namespace :: ops](https://github.com/EvionGit/VoIP_Prototype-opus/tree/main/include/ops),[namespace :: jbuf](https://github.com/EvionGit/VoIP_Prototype-opus/tree/main/include/jbuf) respectively***</ins>
<br><br><br>
___

### Module 4. *Controller*
The [VoIP](https://github.com/EvionGit/VoIP_Prototype-opus/blob/main/include/VoIP.h) class defines methods for rendering the interface and working with program states.
During the operation of the application, the controller controls the encoder and decoder. Sends configuration packets and receives control signals.
<br><br>
Below is a use case diagram.
![](https://github.com/EvionGit/VoIP_Prototype-opus/blob/main/readme-src/VoIP_UML_USE_CASE.png)
<br><br><br>
____

## Demonstration of the work

+ A start window with a selection of audio capture settings and an active network interface.
  + <img src="https://github.com/EvionGit/VoIP_Prototype-opus/blob/main/readme-src/start-w.png" width="300">

+ The main window with an invitation to enter the IP address and port of the companion. With functional call control buttons.
  + <img src="https://github.com/EvionGit/VoIP_Prototype-opus/blob/main/readme-src/main-w.png" width="300">

+ The settings window allows you to specify the active microphone and the data transmission quality. Additionally, notification sound. There is also an information table of the current network interface.
  + <img src="https://github.com/EvionGit/VoIP_Prototype-opus/blob/main/readme-src/settings-w.png" width="300">

+ Process window with timer.
  + <img src="https://github.com/EvionGit/VoIP_Prototype-opus/blob/main/readme-src/call-w.png" width="300">

<br><br><br>
____

## Ways of development
- [X] build UI-frame
- [X] build controller
- [X] choose audio codec
- [X] binding with controller
- [X] use raw UDP and TCP
- [X] build P2P network model
- [ ] migration to RTP/RTCP/SIP
- [ ] add encryption
- [ ] update jitter buffer to adaptive
- [ ] add conferences
- [ ] add video communication
- [ ] migration to model with dedicated server 

____

### *Note that: for the current version of the application built on the P2P model, calls in the global network require a dedicated IP address or a preliminary port forwarding via NAT* 


