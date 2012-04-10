//==============================================================================
//
// Copyright (C) 2008 by Vinnie Falco, this file is part of VFLib.
// See the file GNU_GPL_v2.txt for full licensing terms.
// 
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or (at your option)
// any later version.
// 
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
// details.
// 
// You should have received a copy of the GNU General Public License along with
// this program; if not, write to the Free Software Foundation, Inc., 51
// Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
// 
//==============================================================================

#ifndef VF_AUDIOSAMPLEBUFFERARRAY_VFHEADER
#define VF_AUDIOSAMPLEBUFFERARRAY_VFHEADER

//==============================================================================
/** 
    A wrapper for an array of pointers to audio data.

    This simple class wraps an array of pointers to multi-channel audio data.
    Provided functions can move the pointers back and forth as a group,
    making certain forms of code that manipulate audio buffers more concise.

    Conversions to and from AudioSampleBuffer allow seamless, intuitive usage.

    @param Channels A template parameter specifying the number of channels in
                    the array of pointers, default to 2 (stereo).

    \ingroup vf_audio
*/

// If AudioSampleBuffer ever becomes a template, we can
// move the Sample typedef into the template parameters.
//
template <int Channels = 2>
class AudioSampleBufferArray
{
public:
  typedef float Sample;

  /** Construct a new array with 0 samples. */
  AudioSampleBufferArray ()
    : m_numSamples (0)
  {
  }

  /** Construct an array from existing data.
  
      @param numSamples      The number of samples in the resulting array. This
                             may be less than or equal to the actual amount of
                             space in the memory pointed to by arrayOfChannels.
      
      @param arrayOfChannels The array of pointers to existing memory.
  */
  AudioSampleBufferArray (int numSamples, Sample* const* arrayOfChannels)
  {
    setFrom (numSamples, arrayOfChannels);
  }

  /** Construct an array from another array.
  
      Both arrays will point to the same memory and contain the same number of
      samples.

      @param other The AudioSampleBufferArray to copy from.
  */
  AudioSampleBufferArray (const AudioSampleBufferArray& other)
  {
    setFrom (other);
  }

  /** Construct an array from an AudioSampleBuffer.

      @param buffer              The AudioSampleBuffer to point to.

      @param startingSampleIndex Zero based index of the first sample to use.

      @param numSamples          The number of samples in the desired range.
                                 This must be less than or equal to the actual
                                 amount of space in the memory pointed to by
                                 buffer, less startingSampleIndex.
  */
  AudioSampleBufferArray (const VF_JUCE::AudioSampleBuffer& buffer,
                          int startSample = 0,
                          int numSamples = -1)
  {
    setFrom (buffer, startSample, numSamples);
  }

  /** Create an array from an AudioSourceChannelInfo.

      @param bufferToFill The AudioSourceChannelInfo to point to.
  */
  AudioSampleBufferArray (const VF_JUCE::AudioSourceChannelInfo& bufferToFill)
  {
    setFrom (*bufferToFill.buffer,
             bufferToFill.startSample,
             bufferToFill.numSamples);
  }

  /** Assign from an AudioSampleBuffer */
  AudioSampleBufferArray& operator= (const VF_JUCE::AudioSampleBuffer& buffer)
  {
    setFrom (buffer);
    return *this;
  }

  /** Assign from an AudioSourceChannelInfo */
  AudioSampleBufferArray& operator= (const VF_JUCE::AudioSourceChannelInfo& bufferToFill)
  {
    setFrom (bufferToFill);
    return *this;
  }

  /** Assign from another array. */
  AudioSampleBufferArray& operator= (const AudioSampleBufferArray& other)
  {
    setFrom (other);
    return *this;
  }

  /** Manually assign a range of samples from a set of pointers. */
  void setFrom (int numSamples, Sample* const* channels)
  {
    m_numSamples = numSamples;
    for (int i=0; i<Channels; i++)
      m_channels[i]=channels[i];
  }

  /** Assign from another array */
  void setFrom (const AudioSampleBufferArray& other)
  {
    m_numSamples = other.m_numSamples;
    for (int i=0; i<Channels; i++)
      m_channels[i] = other.m_channels[i];
  }

  /** Assign from a range within an AudioSampleBuffer  */
  void setFrom (VF_JUCE::AudioSampleBuffer const& buffer,
                int startSample = 0,
                int numSamples = -1)
  {
    jassert (buffer.getNumChannels() == Channels);
    jassert (startSample >= 0 && startSample <= buffer.getNumSamples());
    if (numSamples == -1)
      numSamples = buffer.getNumSamples() - startSample;
    jassert (numSamples <= buffer.getNumSamples() - startSample);
    m_numSamples = numSamples;
    for (int i = 0; i < Channels; ++i)
      m_channels[i] = buffer.getArrayOfChannels()[i] + startSample;
  }

  /** Type conversion to an array of pointers to sample data. */
  operator Sample* const* () const
  {
    return m_channels;
  }

  /** Type conversion to AudioSampleBuffer */
  operator VF_JUCE::AudioSampleBuffer()
  {
    return VF_JUCE::AudioSampleBuffer (m_channels, Channels, m_numSamples);
  }

  operator const AudioSampleBuffer() const
  {
    return VF_JUCE::AudioSampleBuffer (m_channels, Channels, m_numSamples);
  }

  /** Get a raw channel pointer.

      @param index The zero based channel number. This must be less than Channels.

      @return A pointer to the channel data.
  */
  Sample*& operator[] (int index)
  {
    jassert (index >= 0 && index < Channels);
    return m_channels[index];
  }

  const Sample* operator[] (int index) const
  {
    jassert (index >= 0 && index < Channels);
    return m_channels[index];
  }

  /** Advance all channels by the specified number of samples.

      Advancing by more than the number of samples remaining is undefined.
      After the pointers are moved forward, the number of samples remaining
      is adjusted downwards.

      @param numSamples The number of samples to advance by.

      @return An array that points to the new range of samples.
  */
  AudioSampleBufferArray operator+ (int numSamples)
  {
    jassert (numSamples <= m_numSamples);
    Sample* channels[Channels];
    for (int i=0; i<Channels; i++)
      channels[i] = m_channels[i] + numSamples;
    return AudioSampleBufferArray (m_numSamples - numSamples, channels);
  }

  AudioSampleBufferArray& operator+= (int numSamples)
  {
    *this = *this + numSamples;
    return *this;
  }

  /** Rewind all channels by the specified number of samples.

      Rewinding to before the start of the original memory pointers is
      undefined. After the pointers are moved back, the number of samples
      remaining is adjusted upwards.

      @param numSamples The number of samples to rewind by.

      @return An array representing the new range of samples.
  */
  AudioSampleBufferArray operator- (int numSamples)
  {
    return operator+ (-numSamples);
  }

  AudioSampleBufferArray& operator-= (int numSamples)
  {
    return operator+= (-numSamples);
  }

  /** Determine if there are samples remaining in the array.

      @return `true` if there are no more samples in the data pointed
                     to by the array.
  */
  bool isEmpty () const
  {
    return m_numSamples <= 0;
  }

  /** Determine the number of samples remaining in the array.

      @return The number of samples remaining. This will decrease or increase
              as the array is incremented or decremented respectively.
  */
  int getNumSamples () const
  {
    return m_numSamples;
  }

  /** Retrieve the raw array of channel pointers.

      @return An array of pointers to sample data.
  */
  Sample* const* getArrayOfChannels() const
  {
    return m_channels;
  }

  /* DEPRECATED
  Sample* const* operator()() const
  {
    return m_channels;
  }
  */

private:
  int m_numSamples;
  Sample* m_channels[Channels];
};

typedef AudioSampleBufferArray <2> StereoSampleBuffer;

#endif