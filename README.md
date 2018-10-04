# Thesis - NPC Emotional Model
## Description
A model for NPC emotions using architectures from psychology. Emotional states of NPCs change dynamically based on events in the world that NPCs witness using my model. More information can be found on my [portfolio](https://www.aahernandez.net/emotionmodel).

## Controls
'E'&ensp;&ensp;&ensp;-&ensp;&ensp;Will cycle forward through different information about the world. If a NPC is selected, it will cycle forward through  
 &ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;information about the NPC.    
'Q'&ensp;&ensp;&ensp;-&ensp;&ensp;Will cycle backward through different information about the world. If a NPC is selected, it will cycle backward through  
 &ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;information about the NPC.  

## Run
An executable is provided in the [Run_Win32](Emotional Model/Run_Win32) folder.

## Build
The project was built using Visual Studio 17. If you would like to build it yourself, you can open the visual studio [solution](Emotional_Model/EmotionModel.sln) to do so. You must also have a copy of my [engine](https://github.com/aahernandez/Engine) to build it, and the engine and emotional model project must both have the same parent folder like so:

|—parent folder  
&ensp;&ensp;&ensp;&ensp;|—Thesis  
&ensp;&ensp;&ensp;&ensp;|—Engine  
