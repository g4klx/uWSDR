all:		UWSDR

CFLAGS		:= -I../Common -I ../DttSP++ $(CFLAGS)

LIBS		:= $(PALIBS) $(JACKLIBS) -lfftw3f -lusb $(LIBS)

OBJS :=		ConfigChooser.o CWKeyboard.o CWKeyer.o DSPControl.o DTTSPControl.o FreqDial.o FreqDisplay.o FreqKeypad.o InfoBox.o \
		SDRParameters.o SMeter.o SpectrumDisplay.o UWSDRApp.o UWSDRFrame.o UWSDRPreferences.o VoiceKeyboard.o VoiceKeyer.o \
		VolumeDial.o

UWSDR:		$(OBJS)
		$(CC) $(LDFLAGS) -o UWSDR $(OBJS) ../Common/Common.a ../DttSP++/DttSP.a $(LIBS)

ConfigChooser.o:	ConfigChooser.cpp ConfigChooser.h UWSDRDefs.h
		$(CC) $(CFLAGS) -c ConfigChooser.cpp

CWKeyboard.o:	CWKeyboard.cpp CWKeyboard.h UWSDRApp.h UWSDRDefs.h
		$(CC) $(CFLAGS) -c CWKeyboard.cpp

CWKeyer.o:	CWKeyer.cpp CWKeyer.h ../Common/ThreadReader.h UWSDRApp.h UWSDRDefs.h
		$(CC) $(CFLAGS) -c CWKeyer.cpp

DSPControl.o:	DSPControl.cpp DSPControl.h CWKeyer.h ../Common/DataCallback.h ../Common/DataReader.h ../Common/Common.h \
		../Common/DataWriter.h DTTSPControl.h ../Common/RingBuffer.h ../Common/ExternalInterface.h ../Common/ExternalProtocolHandler.h \
		../Common/SoundFileWriter.h UWSDRApp.h UWSDRDefs.h VoiceKeyer.h
		$(CC) $(CFLAGS) -c DSPControl.cpp

DTTSPControl.o:	DTTSPControl.cpp DTTSPControl.h UWSDRDefs.h ../Common/Common.h ../DttSP++/DttSP.h
		$(CC) $(CFLAGS) -c DTTSPControl.cpp

FreqDial.o:	FreqDial.cpp FreqDial.h ../Common/DialCallback.h UWSDRDefs.h
		$(CC) $(CFLAGS) -c FreqDial.cpp

FreqDisplay.o:	FreqDisplay.cpp FreqDisplay.h ../Common/Frequency.h UWSDRDefs.h
		$(CC) $(CFLAGS) -c FreqDisplay.cpp

FreqKeypad.o:	FreqKeypad.cpp FreqKeypad.h ../Common/Frequency.h UWSDRDefs.h
		$(CC) $(CFLAGS) -c FreqKeypad.cpp

InfoBox.o:	InfoBox.cpp InfoBox.h UWSDRDefs.h
		$(CC) $(CFLAGS) -c InfoBox.cpp

SDRParameters.o: SDRParameters.cpp SDRParameters.h UWSDRDefs.h ../Common/AudioDevInfo.h ../Common/Common.h ../Common/Frequency.h \
		../Common/SDRDescrFile.h ../Common/FrequencyRangeArray.h
		$(CC) $(CFLAGS) -c SDRParameters.cpp

SMeter.o:	SMeter.cpp SMeter.h UWSDRDefs.h
		$(CC) $(CFLAGS) -c SMeter.cpp

SpectrumDisplay.o: SpectrumDisplay.cpp SpectrumDisplay.h UWSDRDefs.h ../Common/Common.h
		$(CC) $(CFLAGS) -c SpectrumDisplay.cpp

UWSDRApp.o:	UWSDRApp.cpp UWSDRApp.h ConfigChooser.h ../Common/Log.h SDRParameters.h UWSDRDefs.h UWSDRFrame.h \
		../Common/SDRDescrFile.h
		$(CC) $(CFLAGS) -c UWSDRApp.cpp

UWSDRFrame.o:	UWSDRFrame.cpp UWSDRFrame.h ../Common/ControlInterface.h CWKeyboard.h ../Common/DialCallback.h DSPControl.h FreqDial.h FreqDisplay.h FreqKeypad.h \
		../Common/Frequency.h InfoBox.h ../Common/NullController.h SDRParameters.h \
		SMeter.h ../Common/JackReaderWriter.h ../Common/SoundCardReaderWriter.h ../Common/SoundFileWriter.h SpectrumDisplay.h	UWSDRApp.h \
		UWSDRDefs.h UWSDRPreferences.h UWSDR.xpm ../Common/Common.h ../Common/ExternalControlInterface.h ../Common/ExternalProtocolHandler.h \
		VoiceKeyboard.h ../Common/DialInterface.h ../Common/GriffinPowerMate.h ../Common/SI570Controller.h ../Common/HackRFController.h ../Common/FrequencyRangeArray.h
		$(CC) $(CFLAGS) -c UWSDRFrame.cpp

UWSDRPreferences.o: UWSDRPreferences.cpp UWSDRPreferences.h DSPControl.h SDRParameters.h UWSDRApp.h UWSDRDefs.h
		$(CC) $(CFLAGS) -c UWSDRPreferences.cpp

VoiceKeyboard.o:	VoiceKeyboard.cpp VoiceKeyboard.h UWSDRApp.h UWSDRDefs.h
		$(CC) $(CFLAGS) -c VoiceKeyboard.cpp

VoiceKeyer.o:	VoiceKeyer.cpp VoiceKeyer.h ../Common/DataCallback.h ../Common/DataReader.h UWSDRApp.h UWSDRDefs.h \
		../Common/SoundFileReader.h
		$(CC) $(CFLAGS) -c VoiceKeyer.cpp

VolumeDial.o:	VolumeDial.cpp VolumeDial.h ../Common/DialCallback.h UWSDRDefs.h
		$(CC) $(CFLAGS) -c VolumeDial.cpp

install:	UWSDR
		install -g bin -o root -m 0775 UWSDR $(BINDIR)

clean:
		rm -f core UWSDR *.o *~ *.bak
