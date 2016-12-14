# miso
Micro Signal/Slot implementetation

# Emitting a signal

Use emit signalname(param1, param2, ...);

This will create in turn a temporary `miso::emitter` object that will call the signal handler method of the signal, which in its turn will call the actual slot connected to this signal.

# Calling the signal handler

The `emitter` object created above will call the `delayed_dispatch` method of the signal, which is very uninteresting in the void signals, however in the signals with arguments calling the signal handler requires a few extra steps. Firstly the `signalname(param1, param2, ...)` is being handled by the 

The [Stackoverflow1] shows how to unpack a tuple holding various values of various types to a function with matching paremeter types. This is necessary in order to have a prefect match between the values the signal was created with and the slots that are supposed to get the same values.

# References

[Stackvoerflow1] http://stackoverflow.com/questions/7858817/unpacking-a-tuple-to-call-a-matching-function-pointer