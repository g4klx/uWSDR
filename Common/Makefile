all:		Common.a

CFLAGS :=	-I../DttSP++ $(CFLAGS)

OBJS := 	AudioDevInfo.o ExternalProtocolHandler.o Frequency.o FrequencyRangeArray.o GriffinPowerMate.o HackRFController.o JackReaderWriter.o \
					Log.o NullController.o NullReader.o NullWriter.o RingBuffer.o SDRDescrFile.o SignalReader.o SI570Controller.o SoundCardReaderWriter.o \
					SoundFileReader.o SoundFileWriter.o ThreadReader.o ThreeToneReader.o TwoToneReader.o UDPReaderWriter.o

Common.a:	$(OBJS)
		ar rcs Common.a $(OBJS)

AudioDevInfo.o:	AudioDevInfo.cpp AudioDevInfo.h Common.h
		$(CC) $(CFLAGS) -c AudioDevInfo.cpp

ExternalProtocolHandler.o:	ExternalProtocolHandler.cpp ExternalProtocolHandler.h UDPReaderWriter.h ExternalControlInterface.h DataCallback.h \
														../DttSP++/Upsampler.h
		$(CC) $(CFLAGS) -c ExternalProtocolHandler.cpp

Frequency.o:	Frequency.cpp Frequency.h
		$(CC) $(CFLAGS) -c Frequency.cpp

FrequencyRangeArray.o:	FrequencyRangeArray.cpp FrequencyRangeArray.h Frequency.h
		$(CC) $(CFLAGS) -c FrequencyRangeArray.cpp

GriffinPowerMate.o: GriffinPowerMate.cpp GriffinPowerMate.h DialInterface.h DialCallback.h
		$(CC) $(CFLAGS) -c GriffinPowerMate.cpp

HackRFController.o:	HackRFController.cpp HackRFController.h ControlInterface.h Frequency.h SDRController.h
		$(CC) $(CFLAGS) -c HackRFController.cpp

JackReaderWriter.o: JackReaderWriter.cpp JackReaderWriter.h DataCallback.h DataReader.h DataWriter.h RingBuffer.h
		$(CC) $(CFLAGS) -c JackReaderWriter.cpp

Log.o:		Log.cpp Log.h
		$(CC) $(CFLAGS) -c Log.cpp

NullController.o:	NullController.cpp NullController.h ControlInterface.h Frequency.h SDRController.h
		$(CC) $(CFLAGS) -c NullController.cpp

NullReader.o:	NullReader.cpp NullReader.h ThreadReader.h
		$(CC) $(CFLAGS) -c NullReader.cpp

NullWriter.o:	NullWriter.cpp NullWriter.h DataWriter.h
		$(CC) $(CFLAGS) -c NullWriter.cpp

RingBuffer.o:	RingBuffer.cpp RingBuffer.h
		$(CC) $(CFLAGS) -c RingBuffer.cpp

SDRDescrFile.o:	SDRDescrFile.cpp SDRDescrFile.h FrequencyRangeArray.h Frequency.h
		$(CC) $(CFLAGS) -c SDRDescrFile.cpp

SignalReader.o:	SignalReader.cpp SignalReader.h ThreadReader.h
		$(CC) $(CFLAGS) -c SignalReader.cpp

SI570Controller.o:	SI570Controller.cpp SI570Controller.h ControlInterface.h Frequency.h SDRController.h
		$(CC) $(CFLAGS) -c SI570Controller.cpp

SoundCardReaderWriter.o: SoundCardReaderWriter.cpp SoundCardReaderWriter.h DataCallback.h DataReader.h DataWriter.h RingBuffer.h
		$(CC) $(CFLAGS) -c SoundCardReaderWriter.cpp

SoundFileReader.o: SoundFileReader.cpp SoundFileReader.h ThreadReader.h
		$(CC) $(CFLAGS) -c SoundFileReader.cpp

SoundFileWriter.o: SoundFileWriter.cpp SoundFileWriter.h DataWriter.h
		$(CC) $(CFLAGS) -c SoundFileWriter.cpp

ThreadReader.o:	ThreadReader.cpp ThreadReader.h DataReader.h DataCallback.h
		$(CC) $(CFLAGS) -c ThreadReader.cpp

ThreeToneReader.o:	ThreeToneReader.cpp ThreeToneReader.h ThreadReader.h
		$(CC) $(CFLAGS) -c ThreeToneReader.cpp

TwoToneReader.o:		TwoToneReader.cpp TwoToneReader.h ThreadReader.h
		$(CC) $(CFLAGS) -c TwoToneReader.cpp

UDPReaderWriter.o:	UDPReaderWriter.cpp UDPReaderWriter.h Common.h
		$(CC) $(CFLAGS) -c UDPReaderWriter.cpp

clean:
		rm -f core Common.a *.o *~ *.bak
