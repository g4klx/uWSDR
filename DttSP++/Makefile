all:		DttSP.a

CFLAGS :=	-I../Common $(CFLAGS)

OBJS :=		AGC.o AMDemod.o AMMod.o BlockLMS.o CorrectIQ.o CXB.o DCBlock.o Downsampler.o DttSP.o Equaliser.o FIR.o FMDemod.o FMMod.o FilterOVSV.o \
					LMS.o Meter.o NoiseBlanker.o Oscillator.o RX.o Spectrum.o SpeechProc.o Squelch.o TX.o Window.o SSBDemod.o SSBMod.o Upsampler.o Utils.o

DttSP.a:	$(OBJS)
		ar rcs DttSP.a $(OBJS)

AGC.o:		AGC.cpp AGC.h Complex.h CXB.h Utils.h
		$(CC) $(CFLAGS) -c AGC.cpp

AMDemod.o:	AMDemod.cpp AMDemod.h Complex.h CXB.h Demod.h
		$(CC) $(CFLAGS) -c AMDemod.cpp

AMMod.o:	AMMod.cpp AMMod.h Complex.h CXB.h Mod.h
		$(CC) $(CFLAGS) -c AMMod.cpp

BlockLMS.o:	BlockLMS.cpp BlockLMS.h CXB.h CXOps.h Utils.h
		$(CC) $(CFLAGS) -c BlockLMS.cpp

CorrectIQ.o: 	CorrectIQ.cpp CorrectIQ.h CXB.h
		$(CC) $(CFLAGS) -c CorrectIQ.cpp

CXB.o:		CXB.cpp CXB.h Complex.h CXOps.h
		$(CC) $(CFLAGS) -c CXB.cpp

DCBlock.o:	DCBlock.cpp DCBlock.h Complex.h CXB.h Meter.h Spectrum.h
		$(CC) $(CFLAGS) -c DCBlock.cpp

Downsampler.o:	Downsampler.cpp Downsampler.h FIR.h Complex.h
		$(CC) $(CFLAGS) -c Downsampler.cpp

DttSP.o:	DttSP.cpp DttSP.h Defs.h ../Common/RingBuffer.h RX.h TX.h
		$(CC) $(CFLAGS) -c DttSP.cpp

Equaliser.o:		Equaliser.cpp Equaliser.h Complex.h CXB.h FilterOVSV.h FIR.h
		$(CC) $(CFLAGS) -c Equaliser.cpp

FIR.o:		FIR.cpp FIR.h Complex.h CXB.h Window.h Utils.h
		$(CC) $(CFLAGS) -c FIR.cpp

FMDemod.o:	FMDemod.cpp FMDemod.h Complex.h CXB.h Demod.h Utils.h
		$(CC) $(CFLAGS) -c FMDemod.cpp

FMMod.o:	FMMod.cpp FMMod.h Complex.h CXB.h Mod.h Utils.h
		$(CC) $(CFLAGS) -c FMMod.cpp

FilterOVSV.o: 	FilterOVSV.cpp FilterOVSV.h Complex.h CXB.h CXOps.h FIR.h Utils.h
		$(CC) $(CFLAGS) -c FilterOVSV.cpp

LMS.o:		LMS.cpp LMS.h CXB.h CXOps.h Utils.h
		$(CC) $(CFLAGS) -c LMS.cpp

Meter.o:	Meter.cpp Meter.h CXB.h Utils.h
		$(CC) $(CFLAGS) -c Meter.cpp

NoiseBlanker.o: NoiseBlanker.cpp NoiseBlanker.h CXB.h
		$(CC) $(CFLAGS) -c NoiseBlanker.cpp

Oscillator.o: 	Oscillator.cpp Oscillator.h CXB.h
		$(CC) $(CFLAGS) -c Oscillator.cpp

RX.o:		RX.cpp RX.h AGC.h AMDemod.h BlockLMS.h CorrectIQ.h CXB.h Defs.h Demod.h FilterOVSV.h FMDemod.h \
		LMS.h Meter.h NoiseBlanker.h Oscillator.h Spectrum.h Squelch.h SSBDemod.h
		$(CC) $(CFLAGS) -c RX.cpp

Spectrum.o:	Spectrum.cpp Spectrum.h CXB.h FIR.h Utils.h Window.h
		$(CC) $(CFLAGS) -c Spectrum.cpp

SpeechProc.o: 	SpeechProc.cpp SpeechProc.h Complex.h CXB.h Utils.h
		$(CC) $(CFLAGS) -c SpeechProc.cpp

Squelch.o:	Squelch.cpp Squelch.h CXB.h
		$(CC) $(CFLAGS) -c Squelch.cpp

SSBDemod.o:	SSBDemod.cpp SSBDemod.h Complex.h CXB.h Demod.h
		$(CC) $(CFLAGS) -c SSBDemod.cpp

SSBMod.o:	SSBMod.cpp SSBMod.h Complex.h CXB.h Mod.h
		$(CC) $(CFLAGS) -c SSBMod.cpp

TX.o:		TX.cpp TX.h AGC.h AMMod.h CorrectIQ.h CXB.h DCBlock.h Defs.h FilterOVSV.h FMMod.h \
		Meter.h Mod.h Oscillator.h Spectrum.h SpeechProc.h Squelch.h SSBMod.h Equaliser.h
		$(CC) $(CFLAGS) -c TX.cpp

Upsampler.o:	Upsampler.cpp Upsampler.h FIR.h Complex.h
		$(CC) $(CFLAGS) -c Upsampler.cpp

Utils.o:	Utils.cpp Utils.h CXB.h
		$(CC) $(CFLAGS) -c Utils.cpp

Window.o:	Window.cpp Window.h Utils.h
		$(CC) $(CFLAGS) -c Window.cpp

clean:
		rm -f core DttSP.a *.o *~ *.bak
