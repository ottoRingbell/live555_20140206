#include "ADTSAudioBufferSource.hh"
#include <GroupsockHelper.hh>

static unsigned const samplingFrequencyTable[16] = {
  96000, 88200, 64000, 48000,
  44100, 32000, 24000, 22050,
  16000, 12000, 11025, 8000,
  7350, 0, 0, 0
};

ADTSAudioBufferSource*
ADTSAudioBufferSource::createNew(UsageEnvironment& env, unsigned int vProfile, unsigned int vSmaplingFrequenceIndex, unsigned int vChannels)
{
  do {


    // Get and check the 'profile':
    u_int8_t profile = vProfile; // 2 bits
    if (profile == 3) {
      env.setResultMsg("Bad (reserved) 'profile': 3 in first frame of ADTS file");
      break;
    }

    // Get and check the 'sampling_frequency_index':
    u_int8_t sampling_frequency_index = vSmaplingFrequenceIndex; 
    if (samplingFrequencyTable[sampling_frequency_index] == 0) {
      env.setResultMsg("Bad 'sampling_frequency_index' in first frame of ADTS file");
      break;
    }

    // Get and check the 'channel_configuration':
    u_int8_t channel_configuration = vChannels; // 3 bits

    // If we get here, the frame header was OK.
    // Reset the fid to the beginning of the file:
#ifdef DEBUG
    fprintf(stderr, "Read first frame: profile %d, "
	    "sampling_frequency_index %d => samplingFrequency %d, "
	    "channel_configuration %d\n",
	    profile,
	    sampling_frequency_index, samplingFrequencyTable[sampling_frequency_index],
	    channel_configuration);
#endif
    return new ADTSAudioBufferSource(env, profile,
				   sampling_frequency_index, channel_configuration);
  } while (0);

  // An error occurred:
  return NULL;
}

ADTSAudioBufferSource
::ADTSAudioBufferSource(UsageEnvironment& env, u_int8_t profile,
		      u_int8_t samplingFrequencyIndex, u_int8_t channelConfiguration)
  : FramedSource(env) {
  fSamplingFrequency = samplingFrequencyTable[samplingFrequencyIndex];
  fNumChannels = channelConfiguration == 0 ? 2 : channelConfiguration;
  fuSecsPerFrame
    = (1024/*samples-per-frame*/*1000000) / fSamplingFrequency/*samples-per-second*/;

  // Construct the 'AudioSpecificConfig', and from it, the corresponding ASCII string:
  unsigned char audioSpecificConfig[2];
  u_int8_t const audioObjectType = profile + 1;
  audioSpecificConfig[0] = (audioObjectType<<3) | (samplingFrequencyIndex>>1);
  audioSpecificConfig[1] = (samplingFrequencyIndex<<7) | (channelConfiguration<<3);
  sprintf(fConfigStr, "%02X%02x", audioSpecificConfig[0], audioSpecificConfig[1]);
}

ADTSAudioBufferSource::~ADTSAudioBufferSource() {
  // TODO
}

// Note: We should change the following to use asynchronous file reading, #####
// as we now do with ByteStreamFileSource. #####
void ADTSAudioBufferSource::doGetNextFrame() {

#if 0
    // AAC data with Silence
    unsigned char SilenceAAC[] = \
    {0xFF,0xF9,0x6C,0x40,0x18,0x02,0x40,0x01,0x48,0x20,0x06,0xFA,0x50,0x00,0x00,0x00,\
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,\
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,\
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,\
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,\
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,\
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,\
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,\
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,\
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,\
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,\
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0E};
#else
    // AAC data with background sound
    unsigned char SilenceAAC[] = \
    {0x01,0x2e,0x34,0x18,0xa8,0x91,0xd2,0x4d,0x58,0xa0,0x6b,0x05,0xb0,0x5a,0x51,\
        0x14,0x29,0x07,0xf2,0x9d,0x7e,0xe9,0x8a,0xee,0xa3,0x49,0x8c,0x12,0xcc,0x9d,0x0c,\
        0xae,0x27,0x36,0x2c,0x2a,0x10,0x91,0x25,0x95,0xae,0xcd,0x0c,0x57,0x6c,0xce,0x1a,\
        0x46,0x00,0x7a,0xf0,0x8b,0xd8,0x7f,0x82,0xce,0xc6,0xc6,0x2e,0xb5,0x00,0xf2,0xbf,\
        0x96,0xc7,0x7b,0x16,0x94,0xa3,0xa4,0xce,0x80,0xb7,0x3f,0x05,0x26,0xa2,0xfc,0x19,\
        0x63,0x6b,0x08,0x1a,0xc0,0x4f,0x4f,0x54,0x53,0x04,0xd2,0x4d,0xa0,0x26,0xa9,0x10,\
        0x51,0xd5,0x5e,0x24,0xac,0x00,0xbc,0xcd,0x85,0x8a,0xa2,0x0a,0x02,0xba,0xfa,0x84,\
        0xd6,0x1a,0x42,0xcb,0xd3,0x0a,0x09,0xc8,0x05,0xf3,0x5f,0xd3,0x19,0x42,0x58,0x35,\
        0x74,0xf4,0x72,0xce,0xd2,0xed,0x18,0xce,0x6d,0x42,0x46,0xd1,0x78,0xec,0xba,0x06,\
        0xa9,0x19,0xa1,0xb3,0x78,0xb0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,\
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,\
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x07};
#endif
    
  int numBytesRead = 0;
  
  // TODO: invoke a function to get audio datan and copy the data to fTO
  numBytesRead = sizeof(SilenceAAC);
  memcpy(fTo, SilenceAAC, numBytesRead);

              
  if (numBytesRead < 0) numBytesRead = 0;
  fFrameSize = numBytesRead;

  // Set the 'presentation time':
  if (fPresentationTime.tv_sec == 0 && fPresentationTime.tv_usec == 0) {
    // This is the first frame, so use the current time:
    gettimeofday(&fPresentationTime, NULL);
  } else {
    // Increment by the play time of the previous frame:
    unsigned uSeconds = fPresentationTime.tv_usec + fuSecsPerFrame;
    fPresentationTime.tv_sec += uSeconds/1000000;
    fPresentationTime.tv_usec = uSeconds%1000000;
  }

  fDurationInMicroseconds = fuSecsPerFrame;

  // Switch to another task, and inform the reader that he has data:
  nextTask() = envir().taskScheduler().scheduleDelayedTask(0,
				(TaskFunc*)FramedSource::afterGetting, this);
}
