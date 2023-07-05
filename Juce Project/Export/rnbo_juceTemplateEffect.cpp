/*******************************************************************************************************************
Copyright (c) 2023 Cycling '74

The code that Max generates automatically and that end users are capable of
exporting and using, and any associated documentation files (the “Software”)
is a work of authorship for which Cycling '74 is the author and owner for
copyright purposes.

This Software is dual-licensed either under the terms of the Cycling '74
License for Max-Generated Code for Export, or alternatively under the terms
of the General Public License (GPL) Version 3. You may use the Software
according to either of these licenses as it is most appropriate for your
project on a case-by-case basis (proprietary or not).

A) Cycling '74 License for Max-Generated Code for Export

A license is hereby granted, free of charge, to any person obtaining a copy
of the Software (“Licensee”) to use, copy, modify, merge, publish, and
distribute copies of the Software, and to permit persons to whom the Software
is furnished to do so, subject to the following conditions:

The Software is licensed to Licensee for all uses that do not include the sale,
sublicensing, or commercial distribution of software that incorporates this
source code. This means that the Licensee is free to use this software for
educational, research, and prototyping purposes, to create musical or other
creative works with software that incorporates this source code, or any other
use that does not constitute selling software that makes use of this source
code. Commercial distribution also includes the packaging of free software with
other paid software, hardware, or software-provided commercial services.

For entities with UNDER $200k in annual revenue or funding, a license is hereby
granted, free of charge, for the sale, sublicensing, or commercial distribution
of software that incorporates this source code, for as long as the entity's
annual revenue remains below $200k annual revenue or funding.

For entities with OVER $200k in annual revenue or funding interested in the
sale, sublicensing, or commercial distribution of software that incorporates
this source code, please send inquiries to licensing@cycling74.com.

The above copyright notice and this license shall be included in all copies or
substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

Please see
https://support.cycling74.com/hc/en-us/articles/10730637742483-RNBO-Export-Licensing-FAQ
for additional information

B) General Public License Version 3 (GPLv3)
Details of the GPLv3 license can be found at: https://www.gnu.org/licenses/gpl-3.0.html
*******************************************************************************************************************/

#include "RNBO_Common.h"
#include "RNBO_AudioSignal.h"

namespace RNBO {


#define floor(x) ((long)(x))

#if defined(__GNUC__) || defined(__clang__)
    #define RNBO_RESTRICT __restrict__
#elif defined(_MSC_VER)
    #define RNBO_RESTRICT __restrict
#endif

#define FIXEDSIZEARRAYINIT(...) { }

class rnbomatic : public PatcherInterfaceImpl {
public:

rnbomatic()
{
}

~rnbomatic()
{
}

rnbomatic* getTopLevelPatcher() {
    return this;
}

void cancelClockEvents()
{
}

template <typename T> void listquicksort(T& arr, T& sortindices, Int l, Int h, bool ascending) {
    if (l < h) {
        Int p = (Int)(this->listpartition(arr, sortindices, l, h, ascending));
        this->listquicksort(arr, sortindices, l, p - 1, ascending);
        this->listquicksort(arr, sortindices, p + 1, h, ascending);
    }
}

template <typename T> Int listpartition(T& arr, T& sortindices, Int l, Int h, bool ascending) {
    number x = arr[(Index)h];
    Int i = (Int)(l - 1);

    for (Int j = (Int)(l); j <= h - 1; j++) {
        bool asc = (bool)((bool)(ascending) && arr[(Index)j] <= x);
        bool desc = (bool)((bool)(!(bool)(ascending)) && arr[(Index)j] >= x);

        if ((bool)(asc) || (bool)(desc)) {
            i++;
            this->listswapelements(arr, i, j);
            this->listswapelements(sortindices, i, j);
        }
    }

    i++;
    this->listswapelements(arr, i, h);
    this->listswapelements(sortindices, i, h);
    return i;
}

template <typename T> void listswapelements(T& arr, Int a, Int b) {
    auto tmp = arr[(Index)a];
    arr[(Index)a] = arr[(Index)b];
    arr[(Index)b] = tmp;
}

inline number linearinterp(number frac, number x, number y) {
    return x + (y - x) * frac;
}

inline number cubicinterp(number a, number w, number x, number y, number z) {
    number a2 = a * a;
    number f0 = z - y - w + x;
    number f1 = w - x - f0;
    number f2 = y - w;
    number f3 = x;
    return f0 * a * a2 + f1 * a2 + f2 * a + f3;
}

inline number splineinterp(number a, number w, number x, number y, number z) {
    number a2 = a * a;
    number f0 = -0.5 * w + 1.5 * x - 1.5 * y + 0.5 * z;
    number f1 = w - 2.5 * x + 2 * y - 0.5 * z;
    number f2 = -0.5 * w + 0.5 * y;
    return f0 * a * a2 + f1 * a2 + f2 * a + x;
}

inline number cosT8(number r) {
    number t84 = 56.0;
    number t83 = 1680.0;
    number t82 = 20160.0;
    number t81 = 2.4801587302e-05;
    number t73 = 42.0;
    number t72 = 840.0;
    number t71 = 1.9841269841e-04;

    if (r < 0.785398163397448309615660845819875721 && r > -0.785398163397448309615660845819875721) {
        number rr = r * r;
        return 1.0 - rr * t81 * (t82 - rr * (t83 - rr * (t84 - rr)));
    } else if (r > 0.0) {
        r -= 1.57079632679489661923132169163975144;
        number rr = r * r;
        return -r * (1.0 - t71 * rr * (t72 - rr * (t73 - rr)));
    } else {
        r += 1.57079632679489661923132169163975144;
        number rr = r * r;
        return r * (1.0 - t71 * rr * (t72 - rr * (t73 - rr)));
    }
}

inline number cosineinterp(number frac, number x, number y) {
    number a2 = (1.0 - this->cosT8(frac * 3.14159265358979323846)) / (number)2.0;
    return x * (1.0 - a2) + y * a2;
}

number mstosamps(MillisecondTime ms) {
    return ms * this->sr * 0.001;
}

number samplerate() {
    return this->sr;
}

Index vectorsize() {
    return this->vs;
}

number maximum(number x, number y) {
    return (x < y ? y : x);
}

inline number safemod(number f, number m) {
    if (m != 0) {
        if (m < 0) {
            m = -m;
        }

        if (f >= m) {
            if (f >= m * 2.0) {
                number d = f / m;
                int i = (int)(rnbo_trunc(d));
                d = d - i;
                f = d * m;
            } else {
                f -= m;
            }
        } else if (f <= -m) {
            if (f <= -m * 2.0) {
                number d = f / m;
                int i = (int)(rnbo_trunc(d));
                d = d - i;
                f = d * m;
            } else {
                f += m;
            }
        }
    } else {
        f = 0.0;
    }

    return f;
}

inline number safesqrt(number num) {
    return (num > 0.0 ? rnbo_sqrt(num) : 0.0);
}

MillisecondTime currenttime() {
    return this->_currentTime;
}

number tempo() {
    return this->getTopLevelPatcher()->globaltransport_getTempo(this->currenttime());
}

number mstobeats(number ms) {
    return ms * this->tempo() * 0.008 / (number)480;
}

MillisecondTime sampstoms(number samps) {
    return samps * 1000 / this->sr;
}

Index getNumMidiInputPorts() const {
    return 0;
}

void processMidiEvent(MillisecondTime , int , ConstByteArray , Index ) {}

Index getNumMidiOutputPorts() const {
    return 0;
}

void process(
    const SampleValue * const* inputs,
    Index numInputs,
    SampleValue * const* outputs,
    Index numOutputs,
    Index n
) {
    this->vs = n;
    this->updateTime(this->getEngine()->getCurrentTime());
    SampleValue * out1 = (numOutputs >= 1 && outputs[0] ? outputs[0] : this->dummyBuffer);
    SampleValue * out2 = (numOutputs >= 2 && outputs[1] ? outputs[1] : this->dummyBuffer);
    const SampleValue * in1 = (numInputs >= 1 && inputs[0] ? inputs[0] : this->zeroBuffer);
    const SampleValue * in2 = (numInputs >= 2 && inputs[1] ? inputs[1] : this->zeroBuffer);
    this->mstosamps_tilde_01_perform(this->mstosamps_tilde_01_ms, this->signals[0], n);
    this->mstosamps_tilde_02_perform(this->mstosamps_tilde_02_ms, this->signals[1], n);
    this->feedbackreader_01_perform(this->signals[2], n);
    this->dspexpr_02_perform(this->signals[2], this->dspexpr_02_in2, this->signals[3], n);
    this->delaytilde_02_perform(this->signals[1], this->signals[3], this->signals[2], n);

    this->pan_tilde_02_perform(
        this->pan_tilde_02_pos,
        this->signals[2],
        this->signals[1],
        this->dummyBuffer,
        n
    );

    this->dspexpr_03_perform(in2, this->signals[1], this->dspexpr_03_in3, out2, n);
    this->feedbackwriter_02_perform(this->signals[2], n);
    this->feedbackreader_02_perform(this->signals[2], n);
    this->dspexpr_04_perform(this->signals[2], this->dspexpr_04_in2, this->signals[1], n);
    this->signaladder_01_perform(this->signals[1], in1, in2, this->signals[1], n);
    this->delaytilde_01_perform(this->signals[0], this->signals[1], this->signals[2], n);

    this->pan_tilde_01_perform(
        this->pan_tilde_01_pos,
        this->signals[2],
        this->signals[0],
        this->dummyBuffer,
        n
    );

    this->dspexpr_01_perform(in1, this->signals[0], this->dspexpr_01_in3, out1, n);
    this->feedbackwriter_01_perform(this->signals[2], n);
    this->stackprotect_perform(n);
    this->globaltransport_advance();
    this->audioProcessSampleCount += this->vs;
}

void prepareToProcess(number sampleRate, Index maxBlockSize, bool force) {
    if (this->maxvs < maxBlockSize || !this->didAllocateSignals) {
        Index i;

        for (i = 0; i < 4; i++) {
            this->signals[i] = resizeSignal(this->signals[i], this->maxvs, maxBlockSize);
        }

        this->feedbacktilde_01_feedbackbuffer = resizeSignal(this->feedbacktilde_01_feedbackbuffer, this->maxvs, maxBlockSize);
        this->feedbacktilde_02_feedbackbuffer = resizeSignal(this->feedbacktilde_02_feedbackbuffer, this->maxvs, maxBlockSize);
        this->globaltransport_tempo = resizeSignal(this->globaltransport_tempo, this->maxvs, maxBlockSize);
        this->globaltransport_state = resizeSignal(this->globaltransport_state, this->maxvs, maxBlockSize);
        this->zeroBuffer = resizeSignal(this->zeroBuffer, this->maxvs, maxBlockSize);
        this->dummyBuffer = resizeSignal(this->dummyBuffer, this->maxvs, maxBlockSize);
        this->didAllocateSignals = true;
    }

    const bool sampleRateChanged = sampleRate != this->sr;
    const bool maxvsChanged = maxBlockSize != this->maxvs;
    const bool forceDSPSetup = sampleRateChanged || maxvsChanged || force;

    if (sampleRateChanged || maxvsChanged) {
        this->vs = maxBlockSize;
        this->maxvs = maxBlockSize;
        this->sr = sampleRate;
        this->invsr = 1 / sampleRate;
    }

    this->delaytilde_02_dspsetup(forceDSPSetup);
    this->delaytilde_01_dspsetup(forceDSPSetup);
    this->globaltransport_dspsetup(forceDSPSetup);

    if (sampleRateChanged)
        this->onSampleRateChanged(sampleRate);
}

void setProbingTarget(MessageTag id) {
    switch (id) {
    default:
        {
        this->setProbingIndex(-1);
        break;
        }
    }
}

void setProbingIndex(ProbingIndex ) {}

Index getProbingChannels(MessageTag outletId) const {
    RNBO_UNUSED(outletId);
    return 0;
}

DataRef* getDataRef(DataRefIndex index)  {
    switch (index) {
    case 0:
        {
        return addressOf(this->delaytilde_01_del_bufferobj);
        break;
        }
    case 1:
        {
        return addressOf(this->delaytilde_02_del_bufferobj);
        break;
        }
    default:
        {
        return nullptr;
        }
    }
}

DataRefIndex getNumDataRefs() const {
    return 2;
}

void fillDataRef(DataRefIndex , DataRef& ) {}

void zeroDataRef(DataRef& ref) {
    ref->setZero();
}

void processDataViewUpdate(DataRefIndex index, MillisecondTime time) {
    this->updateTime(time);

    if (index == 0) {
        this->delaytilde_01_del_buffer = new Float64Buffer(this->delaytilde_01_del_bufferobj);
    }

    if (index == 1) {
        this->delaytilde_02_del_buffer = new Float64Buffer(this->delaytilde_02_del_bufferobj);
    }
}

void initialize() {
    this->delaytilde_01_del_bufferobj = initDataRef("delaytilde_01_del_bufferobj", true, nullptr, "buffer~");
    this->delaytilde_02_del_bufferobj = initDataRef("delaytilde_02_del_bufferobj", true, nullptr, "buffer~");
    this->assign_defaults();
    this->setState();
    this->delaytilde_01_del_bufferobj->setIndex(0);
    this->delaytilde_01_del_buffer = new Float64Buffer(this->delaytilde_01_del_bufferobj);
    this->delaytilde_02_del_bufferobj->setIndex(1);
    this->delaytilde_02_del_buffer = new Float64Buffer(this->delaytilde_02_del_bufferobj);
    this->initializeObjects();
    this->allocateDataRefs();
    this->startup();
}

Index getIsMuted()  {
    return this->isMuted;
}

void setIsMuted(Index v)  {
    this->isMuted = v;
}

Index getPatcherSerial() const {
    return 0;
}

void getState(PatcherStateInterface& ) {}

void setState() {}

void getPreset(PatcherStateInterface& preset) {
    preset["__presetid"] = "rnbo";
    this->param_01_getPresetValue(getSubState(preset, "delaytimeL"));
    this->param_02_getPresetValue(getSubState(preset, "feedbackL"));
    this->param_03_getPresetValue(getSubState(preset, "dryWetMix"));
    this->param_04_getPresetValue(getSubState(preset, "delaytimeR"));
    this->param_05_getPresetValue(getSubState(preset, "feedbackR"));
}

void setPreset(MillisecondTime time, PatcherStateInterface& preset) {
    this->updateTime(time);
    this->param_01_setPresetValue(getSubState(preset, "delaytimeL"));
    this->param_02_setPresetValue(getSubState(preset, "feedbackL"));
    this->param_03_setPresetValue(getSubState(preset, "dryWetMix"));
    this->param_04_setPresetValue(getSubState(preset, "delaytimeR"));
    this->param_05_setPresetValue(getSubState(preset, "feedbackR"));
}

void processTempoEvent(MillisecondTime time, Tempo tempo) {
    this->updateTime(time);

    if (this->globaltransport_setTempo(this->_currentTime, tempo, false))
        {}
}

void processTransportEvent(MillisecondTime time, TransportState state) {
    this->updateTime(time);

    if (this->globaltransport_setState(this->_currentTime, state, false))
        {}
}

void processBeatTimeEvent(MillisecondTime time, BeatTime beattime) {
    this->updateTime(time);

    if (this->globaltransport_setBeatTime(this->_currentTime, beattime, false))
        {}
}

void onSampleRateChanged(double ) {}

void processTimeSignatureEvent(MillisecondTime time, int numerator, int denominator) {
    this->updateTime(time);

    if (this->globaltransport_setTimeSignature(this->_currentTime, numerator, denominator, false))
        {}
}

void setParameterValue(ParameterIndex index, ParameterValue v, MillisecondTime time) {
    this->updateTime(time);

    switch (index) {
    case 0:
        {
        this->param_01_value_set(v);
        break;
        }
    case 1:
        {
        this->param_02_value_set(v);
        break;
        }
    case 2:
        {
        this->param_03_value_set(v);
        break;
        }
    case 3:
        {
        this->param_04_value_set(v);
        break;
        }
    case 4:
        {
        this->param_05_value_set(v);
        break;
        }
    }
}

void processParameterEvent(ParameterIndex index, ParameterValue value, MillisecondTime time) {
    this->setParameterValue(index, value, time);
}

void processNormalizedParameterEvent(ParameterIndex index, ParameterValue value, MillisecondTime time) {
    this->setParameterValueNormalized(index, value, time);
}

ParameterValue getParameterValue(ParameterIndex index)  {
    switch (index) {
    case 0:
        {
        return this->param_01_value;
        }
    case 1:
        {
        return this->param_02_value;
        }
    case 2:
        {
        return this->param_03_value;
        }
    case 3:
        {
        return this->param_04_value;
        }
    case 4:
        {
        return this->param_05_value;
        }
    default:
        {
        return 0;
        }
    }
}

ParameterIndex getNumSignalInParameters() const {
    return 0;
}

ParameterIndex getNumSignalOutParameters() const {
    return 0;
}

ParameterIndex getNumParameters() const {
    return 5;
}

ConstCharPointer getParameterName(ParameterIndex index) const {
    switch (index) {
    case 0:
        {
        return "delaytimeL";
        }
    case 1:
        {
        return "feedbackL";
        }
    case 2:
        {
        return "dryWetMix";
        }
    case 3:
        {
        return "delaytimeR";
        }
    case 4:
        {
        return "feedbackR";
        }
    default:
        {
        return "bogus";
        }
    }
}

ConstCharPointer getParameterId(ParameterIndex index) const {
    switch (index) {
    case 0:
        {
        return "delaytimeL";
        }
    case 1:
        {
        return "feedbackL";
        }
    case 2:
        {
        return "dryWetMix";
        }
    case 3:
        {
        return "delaytimeR";
        }
    case 4:
        {
        return "feedbackR";
        }
    default:
        {
        return "bogus";
        }
    }
}

void getParameterInfo(ParameterIndex index, ParameterInfo * info) const {
    {
        switch (index) {
        case 0:
            {
            info->type = ParameterTypeNumber;
            info->initialValue = 1;
            info->min = 1;
            info->max = 2000;
            info->exponent = 1;
            info->steps = 0;
            info->debug = false;
            info->saveable = true;
            info->transmittable = true;
            info->initialized = true;
            info->visible = true;
            info->displayName = "";
            info->unit = "";
            info->ioType = IOTypeUndefined;
            info->signalIndex = INVALID_INDEX;
            break;
            }
        case 1:
            {
            info->type = ParameterTypeNumber;
            info->initialValue = 0.5;
            info->min = 0;
            info->max = 0.999;
            info->exponent = 1;
            info->steps = 0;
            info->debug = false;
            info->saveable = true;
            info->transmittable = true;
            info->initialized = true;
            info->visible = true;
            info->displayName = "";
            info->unit = "";
            info->ioType = IOTypeUndefined;
            info->signalIndex = INVALID_INDEX;
            break;
            }
        case 2:
            {
            info->type = ParameterTypeNumber;
            info->initialValue = 0.2;
            info->min = 0;
            info->max = 1;
            info->exponent = 1;
            info->steps = 0;
            info->debug = false;
            info->saveable = true;
            info->transmittable = true;
            info->initialized = true;
            info->visible = true;
            info->displayName = "";
            info->unit = "";
            info->ioType = IOTypeUndefined;
            info->signalIndex = INVALID_INDEX;
            break;
            }
        case 3:
            {
            info->type = ParameterTypeNumber;
            info->initialValue = 1;
            info->min = 1;
            info->max = 2000;
            info->exponent = 1;
            info->steps = 0;
            info->debug = false;
            info->saveable = true;
            info->transmittable = true;
            info->initialized = true;
            info->visible = true;
            info->displayName = "";
            info->unit = "";
            info->ioType = IOTypeUndefined;
            info->signalIndex = INVALID_INDEX;
            break;
            }
        case 4:
            {
            info->type = ParameterTypeNumber;
            info->initialValue = 0.5;
            info->min = 0;
            info->max = 0.999;
            info->exponent = 1;
            info->steps = 0;
            info->debug = false;
            info->saveable = true;
            info->transmittable = true;
            info->initialized = true;
            info->visible = true;
            info->displayName = "";
            info->unit = "";
            info->ioType = IOTypeUndefined;
            info->signalIndex = INVALID_INDEX;
            break;
            }
        }
    }
}

void sendParameter(ParameterIndex index, bool ignoreValue) {
    this->getEngine()->notifyParameterValueChanged(index, (ignoreValue ? 0 : this->getParameterValue(index)), ignoreValue);
}

ParameterValue applyStepsToNormalizedParameterValue(ParameterValue normalizedValue, int steps) const {
    if (steps == 1) {
        if (normalizedValue > 0) {
            normalizedValue = 1.;
        }
    } else {
        ParameterValue oneStep = (number)1. / (steps - 1);
        ParameterValue numberOfSteps = rnbo_fround(normalizedValue / oneStep * 1 / (number)1) * (number)1;
        normalizedValue = numberOfSteps * oneStep;
    }

    return normalizedValue;
}

ParameterValue convertToNormalizedParameterValue(ParameterIndex index, ParameterValue value) const {
    switch (index) {
    case 2:
        {
        {
            value = (value < 0 ? 0 : (value > 1 ? 1 : value));
            ParameterValue normalizedValue = (value - 0) / (1 - 0);
            return normalizedValue;
        }
        }
    case 1:
    case 4:
        {
        {
            value = (value < 0 ? 0 : (value > 0.999 ? 0.999 : value));
            ParameterValue normalizedValue = (value - 0) / (0.999 - 0);
            return normalizedValue;
        }
        }
    case 0:
    case 3:
        {
        {
            value = (value < 1 ? 1 : (value > 2000 ? 2000 : value));
            ParameterValue normalizedValue = (value - 1) / (2000 - 1);
            return normalizedValue;
        }
        }
    default:
        {
        return value;
        }
    }
}

ParameterValue convertFromNormalizedParameterValue(ParameterIndex index, ParameterValue value) const {
    value = (value < 0 ? 0 : (value > 1 ? 1 : value));

    switch (index) {
    case 2:
        {
        {
            value = (value < 0 ? 0 : (value > 1 ? 1 : value));

            {
                return 0 + value * (1 - 0);
            }
        }
        }
    case 1:
    case 4:
        {
        {
            value = (value < 0 ? 0 : (value > 1 ? 1 : value));

            {
                return 0 + value * (0.999 - 0);
            }
        }
        }
    case 0:
    case 3:
        {
        {
            value = (value < 0 ? 0 : (value > 1 ? 1 : value));

            {
                return 1 + value * (2000 - 1);
            }
        }
        }
    default:
        {
        return value;
        }
    }
}

ParameterValue constrainParameterValue(ParameterIndex index, ParameterValue value) const {
    switch (index) {
    case 0:
        {
        return this->param_01_value_constrain(value);
        }
    case 1:
        {
        return this->param_02_value_constrain(value);
        }
    case 2:
        {
        return this->param_03_value_constrain(value);
        }
    case 3:
        {
        return this->param_04_value_constrain(value);
        }
    case 4:
        {
        return this->param_05_value_constrain(value);
        }
    default:
        {
        return value;
        }
    }
}

void scheduleParamInit(ParameterIndex index, Index order) {
    this->paramInitIndices->push(index);
    this->paramInitOrder->push(order);
}

void processParamInitEvents() {
    this->listquicksort(
        this->paramInitOrder,
        this->paramInitIndices,
        0,
        (int)(this->paramInitOrder->length - 1),
        true
    );

    for (Index i = 0; i < this->paramInitOrder->length; i++) {
        this->getEngine()->scheduleParameterChange(
            this->paramInitIndices[i],
            this->getParameterValue(this->paramInitIndices[i]),
            0
        );
    }
}

void processClockEvent(MillisecondTime , ClockId , bool , ParameterValue ) {}

void processOutletAtCurrentTime(EngineLink* , OutletIndex , ParameterValue ) {}

void processOutletEvent(
    EngineLink* sender,
    OutletIndex index,
    ParameterValue value,
    MillisecondTime time
) {
    this->updateTime(time);
    this->processOutletAtCurrentTime(sender, index, value);
}

void processNumMessage(MessageTag , MessageTag , MillisecondTime , number ) {}

void processListMessage(MessageTag , MessageTag , MillisecondTime , const list& ) {}

void processBangMessage(MessageTag , MessageTag , MillisecondTime ) {}

MessageTagInfo resolveTag(MessageTag tag) const {
    switch (tag) {

    }

    return "";
}

MessageIndex getNumMessages() const {
    return 0;
}

const MessageInfo& getMessageInfo(MessageIndex index) const {
    switch (index) {

    }

    return NullMessageInfo;
}

protected:

void param_01_value_set(number v) {
    v = this->param_01_value_constrain(v);
    this->param_01_value = v;
    this->sendParameter(0, false);

    if (this->param_01_value != this->param_01_lastValue) {
        this->getEngine()->presetTouched();
        this->param_01_lastValue = this->param_01_value;
    }

    this->mstosamps_tilde_01_ms_set(v);
}

void param_02_value_set(number v) {
    v = this->param_02_value_constrain(v);
    this->param_02_value = v;
    this->sendParameter(1, false);

    if (this->param_02_value != this->param_02_lastValue) {
        this->getEngine()->presetTouched();
        this->param_02_lastValue = this->param_02_value;
    }

    this->dspexpr_02_in2_set(v);
}

void param_03_value_set(number v) {
    v = this->param_03_value_constrain(v);
    this->param_03_value = v;
    this->sendParameter(2, false);

    if (this->param_03_value != this->param_03_lastValue) {
        this->getEngine()->presetTouched();
        this->param_03_lastValue = this->param_03_value;
    }

    this->dspexpr_03_in3_set(v);
    this->dspexpr_01_in3_set(v);
}

void param_04_value_set(number v) {
    v = this->param_04_value_constrain(v);
    this->param_04_value = v;
    this->sendParameter(3, false);

    if (this->param_04_value != this->param_04_lastValue) {
        this->getEngine()->presetTouched();
        this->param_04_lastValue = this->param_04_value;
    }

    this->mstosamps_tilde_02_ms_set(v);
}

void param_05_value_set(number v) {
    v = this->param_05_value_constrain(v);
    this->param_05_value = v;
    this->sendParameter(4, false);

    if (this->param_05_value != this->param_05_lastValue) {
        this->getEngine()->presetTouched();
        this->param_05_lastValue = this->param_05_value;
    }

    this->dspexpr_04_in2_set(v);
}

number msToSamps(MillisecondTime ms, number sampleRate) {
    return ms * sampleRate * 0.001;
}

MillisecondTime sampsToMs(SampleIndex samps) {
    return samps * (this->invsr * 1000);
}

Index getMaxBlockSize() const {
    return this->maxvs;
}

number getSampleRate() const {
    return this->sr;
}

bool hasFixedVectorSize() const {
    return false;
}

Index getNumInputChannels() const {
    return 2;
}

Index getNumOutputChannels() const {
    return 2;
}

void allocateDataRefs() {
    this->delaytilde_01_del_buffer = this->delaytilde_01_del_buffer->allocateIfNeeded();

    if (this->delaytilde_01_del_bufferobj->hasRequestedSize()) {
        if (this->delaytilde_01_del_bufferobj->wantsFill())
            this->zeroDataRef(this->delaytilde_01_del_bufferobj);

        this->getEngine()->sendDataRefUpdated(0);
    }

    this->delaytilde_02_del_buffer = this->delaytilde_02_del_buffer->allocateIfNeeded();

    if (this->delaytilde_02_del_bufferobj->hasRequestedSize()) {
        if (this->delaytilde_02_del_bufferobj->wantsFill())
            this->zeroDataRef(this->delaytilde_02_del_bufferobj);

        this->getEngine()->sendDataRefUpdated(1);
    }
}

void initializeObjects() {
    this->delaytilde_01_del_init();
    this->delaytilde_02_del_init();
}

void sendOutlet(OutletIndex index, ParameterValue value) {
    this->getEngine()->sendOutlet(this, index, value);
}

void startup() {
    this->updateTime(this->getEngine()->getCurrentTime());

    {
        this->scheduleParamInit(0, 0);
    }

    {
        this->scheduleParamInit(1, 0);
    }

    {
        this->scheduleParamInit(2, 0);
    }

    {
        this->scheduleParamInit(3, 0);
    }

    {
        this->scheduleParamInit(4, 0);
    }

    this->processParamInitEvents();
}

static number param_01_value_constrain(number v) {
    v = (v > 2000 ? 2000 : (v < 1 ? 1 : v));
    return v;
}

void mstosamps_tilde_01_ms_set(number v) {
    this->mstosamps_tilde_01_ms = v;
}

static number param_02_value_constrain(number v) {
    v = (v > 0.999 ? 0.999 : (v < 0 ? 0 : v));
    return v;
}

void dspexpr_02_in2_set(number v) {
    this->dspexpr_02_in2 = v;
}

static number param_03_value_constrain(number v) {
    v = (v > 1 ? 1 : (v < 0 ? 0 : v));
    return v;
}

void dspexpr_03_in3_set(number v) {
    this->dspexpr_03_in3 = v;
}

void dspexpr_01_in3_set(number v) {
    this->dspexpr_01_in3 = v;
}

static number param_04_value_constrain(number v) {
    v = (v > 2000 ? 2000 : (v < 1 ? 1 : v));
    return v;
}

void mstosamps_tilde_02_ms_set(number v) {
    this->mstosamps_tilde_02_ms = v;
}

static number param_05_value_constrain(number v) {
    v = (v > 0.999 ? 0.999 : (v < 0 ? 0 : v));
    return v;
}

void dspexpr_04_in2_set(number v) {
    this->dspexpr_04_in2 = v;
}

void mstosamps_tilde_01_perform(number ms, SampleValue * out1, Index n) {
    Index i;

    for (i = 0; i < n; i++) {
        out1[(Index)i] = ms * this->sr * 0.001;
    }
}

void mstosamps_tilde_02_perform(number ms, SampleValue * out1, Index n) {
    Index i;

    for (i = 0; i < n; i++) {
        out1[(Index)i] = ms * this->sr * 0.001;
    }
}

void feedbackreader_01_perform(SampleValue * output, Index n) {
    auto& buffer = this->feedbacktilde_01_feedbackbuffer;

    for (Index i = 0; i < n; i++) {
        output[(Index)i] = buffer[(Index)i];
    }
}

void dspexpr_02_perform(const Sample * in1, number in2, SampleValue * out1, Index n) {
    Index i;

    for (i = 0; i < n; i++) {
        out1[(Index)i] = in1[(Index)i] * in2;//#map:_###_obj_###_:1
    }
}

void delaytilde_02_perform(
    const Sample * delay,
    const SampleValue * input,
    SampleValue * output,
    Index n
) {
    auto __delaytilde_02_crossfadeDelay = this->delaytilde_02_crossfadeDelay;
    auto __delaytilde_02_rampInSamples = this->delaytilde_02_rampInSamples;
    auto __delaytilde_02_ramp = this->delaytilde_02_ramp;
    auto __delaytilde_02_lastDelay = this->delaytilde_02_lastDelay;

    for (Index i = 0; i < n; i++) {
        if (__delaytilde_02_lastDelay == -1) {
            __delaytilde_02_lastDelay = delay[(Index)i];
        }

        if (__delaytilde_02_ramp > 0) {
            number factor = __delaytilde_02_ramp / __delaytilde_02_rampInSamples;
            output[(Index)i] = this->delaytilde_02_del_read(__delaytilde_02_crossfadeDelay, 0) * factor + this->delaytilde_02_del_read(__delaytilde_02_lastDelay, 0) * (1. - factor);
            __delaytilde_02_ramp--;
        } else {
            number effectiveDelay = delay[(Index)i];

            if (effectiveDelay != __delaytilde_02_lastDelay) {
                __delaytilde_02_ramp = __delaytilde_02_rampInSamples;
                __delaytilde_02_crossfadeDelay = __delaytilde_02_lastDelay;
                __delaytilde_02_lastDelay = effectiveDelay;
                output[(Index)i] = this->delaytilde_02_del_read(__delaytilde_02_crossfadeDelay, 0);
                __delaytilde_02_ramp--;
            } else {
                output[(Index)i] = this->delaytilde_02_del_read(effectiveDelay, 0);
            }
        }

        this->delaytilde_02_del_write(input[(Index)i]);
        this->delaytilde_02_del_step();
    }

    this->delaytilde_02_lastDelay = __delaytilde_02_lastDelay;
    this->delaytilde_02_ramp = __delaytilde_02_ramp;
    this->delaytilde_02_crossfadeDelay = __delaytilde_02_crossfadeDelay;
}

void pan_tilde_02_perform(
    number pos,
    const SampleValue * in0,
    SampleValue * out1,
    SampleValue * out2,
    Index n
) {
    RNBO_UNUSED(pos);
    Index i;

    for (i = 0; i < n; i++) {
        out2[(Index)i] = this->pan_tilde_02_func_next(0, 1) * in0[(Index)i];
        out1[(Index)i] = this->pan_tilde_02_func_next(0, 0) * in0[(Index)i];
    }
}

void dspexpr_03_perform(
    const Sample * in1,
    const Sample * in2,
    number in3,
    SampleValue * out1,
    Index n
) {
    Index i;

    for (i = 0; i < n; i++) {
        out1[(Index)i] = in1[(Index)i] + in3 * (in2[(Index)i] - in1[(Index)i]);//#map:_###_obj_###_:1
    }
}

void feedbackwriter_02_perform(const SampleValue * input, Index n) {
    auto& buffer = this->feedbacktilde_02_feedbackbuffer;

    for (Index i = 0; i < n; i++) {
        buffer[(Index)i] = input[(Index)i];
    }
}

void feedbackreader_02_perform(SampleValue * output, Index n) {
    auto& buffer = this->feedbacktilde_02_feedbackbuffer;

    for (Index i = 0; i < n; i++) {
        output[(Index)i] = buffer[(Index)i];
    }
}

void dspexpr_04_perform(const Sample * in1, number in2, SampleValue * out1, Index n) {
    Index i;

    for (i = 0; i < n; i++) {
        out1[(Index)i] = in1[(Index)i] * in2;//#map:_###_obj_###_:1
    }
}

void signaladder_01_perform(
    const SampleValue * in1,
    const SampleValue * in2,
    const SampleValue * in3,
    SampleValue * out,
    Index n
) {
    Index i;

    for (i = 0; i < n; i++) {
        out[(Index)i] = in1[(Index)i] + in2[(Index)i] + in3[(Index)i];
    }
}

void delaytilde_01_perform(
    const Sample * delay,
    const SampleValue * input,
    SampleValue * output,
    Index n
) {
    auto __delaytilde_01_crossfadeDelay = this->delaytilde_01_crossfadeDelay;
    auto __delaytilde_01_rampInSamples = this->delaytilde_01_rampInSamples;
    auto __delaytilde_01_ramp = this->delaytilde_01_ramp;
    auto __delaytilde_01_lastDelay = this->delaytilde_01_lastDelay;

    for (Index i = 0; i < n; i++) {
        if (__delaytilde_01_lastDelay == -1) {
            __delaytilde_01_lastDelay = delay[(Index)i];
        }

        if (__delaytilde_01_ramp > 0) {
            number factor = __delaytilde_01_ramp / __delaytilde_01_rampInSamples;
            output[(Index)i] = this->delaytilde_01_del_read(__delaytilde_01_crossfadeDelay, 0) * factor + this->delaytilde_01_del_read(__delaytilde_01_lastDelay, 0) * (1. - factor);
            __delaytilde_01_ramp--;
        } else {
            number effectiveDelay = delay[(Index)i];

            if (effectiveDelay != __delaytilde_01_lastDelay) {
                __delaytilde_01_ramp = __delaytilde_01_rampInSamples;
                __delaytilde_01_crossfadeDelay = __delaytilde_01_lastDelay;
                __delaytilde_01_lastDelay = effectiveDelay;
                output[(Index)i] = this->delaytilde_01_del_read(__delaytilde_01_crossfadeDelay, 0);
                __delaytilde_01_ramp--;
            } else {
                output[(Index)i] = this->delaytilde_01_del_read(effectiveDelay, 0);
            }
        }

        this->delaytilde_01_del_write(input[(Index)i]);
        this->delaytilde_01_del_step();
    }

    this->delaytilde_01_lastDelay = __delaytilde_01_lastDelay;
    this->delaytilde_01_ramp = __delaytilde_01_ramp;
    this->delaytilde_01_crossfadeDelay = __delaytilde_01_crossfadeDelay;
}

void pan_tilde_01_perform(
    number pos,
    const SampleValue * in0,
    SampleValue * out1,
    SampleValue * out2,
    Index n
) {
    RNBO_UNUSED(pos);
    Index i;

    for (i = 0; i < n; i++) {
        out2[(Index)i] = this->pan_tilde_01_func_next(0, 1) * in0[(Index)i];
        out1[(Index)i] = this->pan_tilde_01_func_next(0, 0) * in0[(Index)i];
    }
}

void dspexpr_01_perform(
    const Sample * in1,
    const Sample * in2,
    number in3,
    SampleValue * out1,
    Index n
) {
    Index i;

    for (i = 0; i < n; i++) {
        out1[(Index)i] = in1[(Index)i] + in3 * (in2[(Index)i] - in1[(Index)i]);//#map:_###_obj_###_:1
    }
}

void feedbackwriter_01_perform(const SampleValue * input, Index n) {
    auto& buffer = this->feedbacktilde_01_feedbackbuffer;

    for (Index i = 0; i < n; i++) {
        buffer[(Index)i] = input[(Index)i];
    }
}

void stackprotect_perform(Index n) {
    RNBO_UNUSED(n);
    auto __stackprotect_count = this->stackprotect_count;
    __stackprotect_count = 0;
    this->stackprotect_count = __stackprotect_count;
}

void delaytilde_01_del_step() {
    this->delaytilde_01_del_reader++;

    if (this->delaytilde_01_del_reader >= (int)(this->delaytilde_01_del_buffer->getSize()))
        this->delaytilde_01_del_reader = 0;
}

number delaytilde_01_del_read(number size, Int interp) {
    if (interp == 0) {
        number r = (int)(this->delaytilde_01_del_buffer->getSize()) + this->delaytilde_01_del_reader - ((size > this->delaytilde_01_del__maxdelay ? this->delaytilde_01_del__maxdelay : (size < (this->delaytilde_01_del_reader != this->delaytilde_01_del_writer) ? this->delaytilde_01_del_reader != this->delaytilde_01_del_writer : size)));
        long index1 = (long)(rnbo_floor(r));
        number frac = r - index1;
        long index2 = (long)(index1 + 1);

        return this->linearinterp(frac, this->delaytilde_01_del_buffer->getSample(
            0,
            (Index)((BinOpInt)((UBinOpInt)index1 & (UBinOpInt)this->delaytilde_01_del_wrap))
        ), this->delaytilde_01_del_buffer->getSample(
            0,
            (Index)((BinOpInt)((UBinOpInt)index2 & (UBinOpInt)this->delaytilde_01_del_wrap))
        ));
    } else if (interp == 1) {
        number r = (int)(this->delaytilde_01_del_buffer->getSize()) + this->delaytilde_01_del_reader - ((size > this->delaytilde_01_del__maxdelay ? this->delaytilde_01_del__maxdelay : (size < (1 + this->delaytilde_01_del_reader != this->delaytilde_01_del_writer) ? 1 + this->delaytilde_01_del_reader != this->delaytilde_01_del_writer : size)));
        long index1 = (long)(rnbo_floor(r));
        number frac = r - index1;
        Index index2 = (Index)(index1 + 1);
        Index index3 = (Index)(index2 + 1);
        Index index4 = (Index)(index3 + 1);

        return this->cubicinterp(frac, this->delaytilde_01_del_buffer->getSample(
            0,
            (Index)((BinOpInt)((UBinOpInt)index1 & (UBinOpInt)this->delaytilde_01_del_wrap))
        ), this->delaytilde_01_del_buffer->getSample(
            0,
            (Index)((BinOpInt)((UBinOpInt)index2 & (UBinOpInt)this->delaytilde_01_del_wrap))
        ), this->delaytilde_01_del_buffer->getSample(
            0,
            (Index)((BinOpInt)((UBinOpInt)index3 & (UBinOpInt)this->delaytilde_01_del_wrap))
        ), this->delaytilde_01_del_buffer->getSample(
            0,
            (Index)((BinOpInt)((UBinOpInt)index4 & (UBinOpInt)this->delaytilde_01_del_wrap))
        ));
    } else if (interp == 2) {
        number r = (int)(this->delaytilde_01_del_buffer->getSize()) + this->delaytilde_01_del_reader - ((size > this->delaytilde_01_del__maxdelay ? this->delaytilde_01_del__maxdelay : (size < (1 + this->delaytilde_01_del_reader != this->delaytilde_01_del_writer) ? 1 + this->delaytilde_01_del_reader != this->delaytilde_01_del_writer : size)));
        long index1 = (long)(rnbo_floor(r));
        number frac = r - index1;
        Index index2 = (Index)(index1 + 1);
        Index index3 = (Index)(index2 + 1);
        Index index4 = (Index)(index3 + 1);

        return this->splineinterp(frac, this->delaytilde_01_del_buffer->getSample(
            0,
            (Index)((BinOpInt)((UBinOpInt)index1 & (UBinOpInt)this->delaytilde_01_del_wrap))
        ), this->delaytilde_01_del_buffer->getSample(
            0,
            (Index)((BinOpInt)((UBinOpInt)index2 & (UBinOpInt)this->delaytilde_01_del_wrap))
        ), this->delaytilde_01_del_buffer->getSample(
            0,
            (Index)((BinOpInt)((UBinOpInt)index3 & (UBinOpInt)this->delaytilde_01_del_wrap))
        ), this->delaytilde_01_del_buffer->getSample(
            0,
            (Index)((BinOpInt)((UBinOpInt)index4 & (UBinOpInt)this->delaytilde_01_del_wrap))
        ));
    } else if (interp == 3) {
        number r = (int)(this->delaytilde_01_del_buffer->getSize()) + this->delaytilde_01_del_reader - ((size > this->delaytilde_01_del__maxdelay ? this->delaytilde_01_del__maxdelay : (size < (this->delaytilde_01_del_reader != this->delaytilde_01_del_writer) ? this->delaytilde_01_del_reader != this->delaytilde_01_del_writer : size)));
        long index1 = (long)(rnbo_floor(r));
        number frac = r - index1;
        Index index2 = (Index)(index1 + 1);

        return this->cosineinterp(frac, this->delaytilde_01_del_buffer->getSample(
            0,
            (Index)((BinOpInt)((UBinOpInt)index1 & (UBinOpInt)this->delaytilde_01_del_wrap))
        ), this->delaytilde_01_del_buffer->getSample(
            0,
            (Index)((BinOpInt)((UBinOpInt)index2 & (UBinOpInt)this->delaytilde_01_del_wrap))
        ));
    }

    number r = (int)(this->delaytilde_01_del_buffer->getSize()) + this->delaytilde_01_del_reader - ((size > this->delaytilde_01_del__maxdelay ? this->delaytilde_01_del__maxdelay : (size < (this->delaytilde_01_del_reader != this->delaytilde_01_del_writer) ? this->delaytilde_01_del_reader != this->delaytilde_01_del_writer : size)));
    long index1 = (long)(rnbo_floor(r));

    return this->delaytilde_01_del_buffer->getSample(
        0,
        (Index)((BinOpInt)((UBinOpInt)index1 & (UBinOpInt)this->delaytilde_01_del_wrap))
    );
}

void delaytilde_01_del_write(number v) {
    this->delaytilde_01_del_writer = this->delaytilde_01_del_reader;
    this->delaytilde_01_del_buffer[(Index)this->delaytilde_01_del_writer] = v;
}

number delaytilde_01_del_next(number v, int size) {
    number effectiveSize = (size == -1 ? this->delaytilde_01_del__maxdelay : size);
    number val = this->delaytilde_01_del_read(effectiveSize, 0);
    this->delaytilde_01_del_write(v);
    this->delaytilde_01_del_step();
    return val;
}

array<Index, 2> delaytilde_01_del_calcSizeInSamples() {
    number sizeInSamples = 0;
    Index allocatedSizeInSamples = 0;

    {
        sizeInSamples = this->delaytilde_01_del_evaluateSizeExpr(this->samplerate(), this->vectorsize());
        this->delaytilde_01_del_sizemode = 0;
    }

    sizeInSamples = rnbo_floor(sizeInSamples);
    sizeInSamples = this->maximum(sizeInSamples, 2);
    allocatedSizeInSamples = (Index)(sizeInSamples);
    allocatedSizeInSamples = nextpoweroftwo(allocatedSizeInSamples);
    return {sizeInSamples, allocatedSizeInSamples};
}

void delaytilde_01_del_init() {
    auto result = this->delaytilde_01_del_calcSizeInSamples();
    this->delaytilde_01_del__maxdelay = result[0];
    Index requestedSizeInSamples = (Index)(result[1]);
    this->delaytilde_01_del_buffer->requestSize(requestedSizeInSamples, 1);
    this->delaytilde_01_del_wrap = requestedSizeInSamples - 1;
}

void delaytilde_01_del_clear() {
    this->delaytilde_01_del_buffer->setZero();
}

void delaytilde_01_del_reset() {
    auto result = this->delaytilde_01_del_calcSizeInSamples();
    this->delaytilde_01_del__maxdelay = result[0];
    Index allocatedSizeInSamples = (Index)(result[1]);
    this->delaytilde_01_del_buffer->setSize(allocatedSizeInSamples);
    updateDataRef(this, this->delaytilde_01_del_buffer);
    this->delaytilde_01_del_wrap = this->delaytilde_01_del_buffer->getSize() - 1;
    this->delaytilde_01_del_clear();

    if (this->delaytilde_01_del_reader >= this->delaytilde_01_del__maxdelay || this->delaytilde_01_del_writer >= this->delaytilde_01_del__maxdelay) {
        this->delaytilde_01_del_reader = 0;
        this->delaytilde_01_del_writer = 0;
    }
}

void delaytilde_01_del_dspsetup() {
    this->delaytilde_01_del_reset();
}

number delaytilde_01_del_evaluateSizeExpr(number samplerate, number vectorsize) {
    RNBO_UNUSED(vectorsize);
    return samplerate * 2;
}

number delaytilde_01_del_size() {
    return this->delaytilde_01_del__maxdelay;
}

void delaytilde_01_dspsetup(bool force) {
    if ((bool)(this->delaytilde_01_setupDone) && (bool)(!(bool)(force)))
        return;

    this->delaytilde_01_rampInSamples = (long)(this->mstosamps(50));
    this->delaytilde_01_lastDelay = -1;
    this->delaytilde_01_setupDone = true;
    this->delaytilde_01_del_dspsetup();
}

number pan_tilde_01_func_next(number pos, int channel) {
    {
        {
            number nchan_1 = 2 - 1;

            {
                pos = pos * nchan_1;
            }

            {
                {
                    {
                        if (pos <= -1 || pos >= 2)
                            return 0;
                    }
                }
            }

            pos = pos - channel;

            if (pos > -1 && pos < 1) {
                {
                    {
                        return this->safesqrt(1.0 - rnbo_abs(pos));
                    }
                }
            } else {
                return 0;
            }
        }
    }
}

void pan_tilde_01_func_reset() {}

void param_01_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->param_01_value;
}

void param_01_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->param_01_value_set(preset["value"]);
}

void param_02_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->param_02_value;
}

void param_02_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->param_02_value_set(preset["value"]);
}

void param_03_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->param_03_value;
}

void param_03_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->param_03_value_set(preset["value"]);
}

void delaytilde_02_del_step() {
    this->delaytilde_02_del_reader++;

    if (this->delaytilde_02_del_reader >= (int)(this->delaytilde_02_del_buffer->getSize()))
        this->delaytilde_02_del_reader = 0;
}

number delaytilde_02_del_read(number size, Int interp) {
    if (interp == 0) {
        number r = (int)(this->delaytilde_02_del_buffer->getSize()) + this->delaytilde_02_del_reader - ((size > this->delaytilde_02_del__maxdelay ? this->delaytilde_02_del__maxdelay : (size < (this->delaytilde_02_del_reader != this->delaytilde_02_del_writer) ? this->delaytilde_02_del_reader != this->delaytilde_02_del_writer : size)));
        long index1 = (long)(rnbo_floor(r));
        number frac = r - index1;
        long index2 = (long)(index1 + 1);

        return this->linearinterp(frac, this->delaytilde_02_del_buffer->getSample(
            0,
            (Index)((BinOpInt)((UBinOpInt)index1 & (UBinOpInt)this->delaytilde_02_del_wrap))
        ), this->delaytilde_02_del_buffer->getSample(
            0,
            (Index)((BinOpInt)((UBinOpInt)index2 & (UBinOpInt)this->delaytilde_02_del_wrap))
        ));
    } else if (interp == 1) {
        number r = (int)(this->delaytilde_02_del_buffer->getSize()) + this->delaytilde_02_del_reader - ((size > this->delaytilde_02_del__maxdelay ? this->delaytilde_02_del__maxdelay : (size < (1 + this->delaytilde_02_del_reader != this->delaytilde_02_del_writer) ? 1 + this->delaytilde_02_del_reader != this->delaytilde_02_del_writer : size)));
        long index1 = (long)(rnbo_floor(r));
        number frac = r - index1;
        Index index2 = (Index)(index1 + 1);
        Index index3 = (Index)(index2 + 1);
        Index index4 = (Index)(index3 + 1);

        return this->cubicinterp(frac, this->delaytilde_02_del_buffer->getSample(
            0,
            (Index)((BinOpInt)((UBinOpInt)index1 & (UBinOpInt)this->delaytilde_02_del_wrap))
        ), this->delaytilde_02_del_buffer->getSample(
            0,
            (Index)((BinOpInt)((UBinOpInt)index2 & (UBinOpInt)this->delaytilde_02_del_wrap))
        ), this->delaytilde_02_del_buffer->getSample(
            0,
            (Index)((BinOpInt)((UBinOpInt)index3 & (UBinOpInt)this->delaytilde_02_del_wrap))
        ), this->delaytilde_02_del_buffer->getSample(
            0,
            (Index)((BinOpInt)((UBinOpInt)index4 & (UBinOpInt)this->delaytilde_02_del_wrap))
        ));
    } else if (interp == 2) {
        number r = (int)(this->delaytilde_02_del_buffer->getSize()) + this->delaytilde_02_del_reader - ((size > this->delaytilde_02_del__maxdelay ? this->delaytilde_02_del__maxdelay : (size < (1 + this->delaytilde_02_del_reader != this->delaytilde_02_del_writer) ? 1 + this->delaytilde_02_del_reader != this->delaytilde_02_del_writer : size)));
        long index1 = (long)(rnbo_floor(r));
        number frac = r - index1;
        Index index2 = (Index)(index1 + 1);
        Index index3 = (Index)(index2 + 1);
        Index index4 = (Index)(index3 + 1);

        return this->splineinterp(frac, this->delaytilde_02_del_buffer->getSample(
            0,
            (Index)((BinOpInt)((UBinOpInt)index1 & (UBinOpInt)this->delaytilde_02_del_wrap))
        ), this->delaytilde_02_del_buffer->getSample(
            0,
            (Index)((BinOpInt)((UBinOpInt)index2 & (UBinOpInt)this->delaytilde_02_del_wrap))
        ), this->delaytilde_02_del_buffer->getSample(
            0,
            (Index)((BinOpInt)((UBinOpInt)index3 & (UBinOpInt)this->delaytilde_02_del_wrap))
        ), this->delaytilde_02_del_buffer->getSample(
            0,
            (Index)((BinOpInt)((UBinOpInt)index4 & (UBinOpInt)this->delaytilde_02_del_wrap))
        ));
    } else if (interp == 3) {
        number r = (int)(this->delaytilde_02_del_buffer->getSize()) + this->delaytilde_02_del_reader - ((size > this->delaytilde_02_del__maxdelay ? this->delaytilde_02_del__maxdelay : (size < (this->delaytilde_02_del_reader != this->delaytilde_02_del_writer) ? this->delaytilde_02_del_reader != this->delaytilde_02_del_writer : size)));
        long index1 = (long)(rnbo_floor(r));
        number frac = r - index1;
        Index index2 = (Index)(index1 + 1);

        return this->cosineinterp(frac, this->delaytilde_02_del_buffer->getSample(
            0,
            (Index)((BinOpInt)((UBinOpInt)index1 & (UBinOpInt)this->delaytilde_02_del_wrap))
        ), this->delaytilde_02_del_buffer->getSample(
            0,
            (Index)((BinOpInt)((UBinOpInt)index2 & (UBinOpInt)this->delaytilde_02_del_wrap))
        ));
    }

    number r = (int)(this->delaytilde_02_del_buffer->getSize()) + this->delaytilde_02_del_reader - ((size > this->delaytilde_02_del__maxdelay ? this->delaytilde_02_del__maxdelay : (size < (this->delaytilde_02_del_reader != this->delaytilde_02_del_writer) ? this->delaytilde_02_del_reader != this->delaytilde_02_del_writer : size)));
    long index1 = (long)(rnbo_floor(r));

    return this->delaytilde_02_del_buffer->getSample(
        0,
        (Index)((BinOpInt)((UBinOpInt)index1 & (UBinOpInt)this->delaytilde_02_del_wrap))
    );
}

void delaytilde_02_del_write(number v) {
    this->delaytilde_02_del_writer = this->delaytilde_02_del_reader;
    this->delaytilde_02_del_buffer[(Index)this->delaytilde_02_del_writer] = v;
}

number delaytilde_02_del_next(number v, int size) {
    number effectiveSize = (size == -1 ? this->delaytilde_02_del__maxdelay : size);
    number val = this->delaytilde_02_del_read(effectiveSize, 0);
    this->delaytilde_02_del_write(v);
    this->delaytilde_02_del_step();
    return val;
}

array<Index, 2> delaytilde_02_del_calcSizeInSamples() {
    number sizeInSamples = 0;
    Index allocatedSizeInSamples = 0;

    {
        sizeInSamples = this->delaytilde_02_del_evaluateSizeExpr(this->samplerate(), this->vectorsize());
        this->delaytilde_02_del_sizemode = 0;
    }

    sizeInSamples = rnbo_floor(sizeInSamples);
    sizeInSamples = this->maximum(sizeInSamples, 2);
    allocatedSizeInSamples = (Index)(sizeInSamples);
    allocatedSizeInSamples = nextpoweroftwo(allocatedSizeInSamples);
    return {sizeInSamples, allocatedSizeInSamples};
}

void delaytilde_02_del_init() {
    auto result = this->delaytilde_02_del_calcSizeInSamples();
    this->delaytilde_02_del__maxdelay = result[0];
    Index requestedSizeInSamples = (Index)(result[1]);
    this->delaytilde_02_del_buffer->requestSize(requestedSizeInSamples, 1);
    this->delaytilde_02_del_wrap = requestedSizeInSamples - 1;
}

void delaytilde_02_del_clear() {
    this->delaytilde_02_del_buffer->setZero();
}

void delaytilde_02_del_reset() {
    auto result = this->delaytilde_02_del_calcSizeInSamples();
    this->delaytilde_02_del__maxdelay = result[0];
    Index allocatedSizeInSamples = (Index)(result[1]);
    this->delaytilde_02_del_buffer->setSize(allocatedSizeInSamples);
    updateDataRef(this, this->delaytilde_02_del_buffer);
    this->delaytilde_02_del_wrap = this->delaytilde_02_del_buffer->getSize() - 1;
    this->delaytilde_02_del_clear();

    if (this->delaytilde_02_del_reader >= this->delaytilde_02_del__maxdelay || this->delaytilde_02_del_writer >= this->delaytilde_02_del__maxdelay) {
        this->delaytilde_02_del_reader = 0;
        this->delaytilde_02_del_writer = 0;
    }
}

void delaytilde_02_del_dspsetup() {
    this->delaytilde_02_del_reset();
}

number delaytilde_02_del_evaluateSizeExpr(number samplerate, number vectorsize) {
    RNBO_UNUSED(vectorsize);
    return samplerate * 2;
}

number delaytilde_02_del_size() {
    return this->delaytilde_02_del__maxdelay;
}

void delaytilde_02_dspsetup(bool force) {
    if ((bool)(this->delaytilde_02_setupDone) && (bool)(!(bool)(force)))
        return;

    this->delaytilde_02_rampInSamples = (long)(this->mstosamps(50));
    this->delaytilde_02_lastDelay = -1;
    this->delaytilde_02_setupDone = true;
    this->delaytilde_02_del_dspsetup();
}

number pan_tilde_02_func_next(number pos, int channel) {
    {
        {
            number nchan_1 = 2 - 1;

            {
                pos = pos * nchan_1;
            }

            {
                {
                    {
                        if (pos <= -1 || pos >= 2)
                            return 0;
                    }
                }
            }

            pos = pos - channel;

            if (pos > -1 && pos < 1) {
                {
                    {
                        return this->safesqrt(1.0 - rnbo_abs(pos));
                    }
                }
            } else {
                return 0;
            }
        }
    }
}

void pan_tilde_02_func_reset() {}

void param_04_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->param_04_value;
}

void param_04_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->param_04_value_set(preset["value"]);
}

void param_05_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->param_05_value;
}

void param_05_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->param_05_value_set(preset["value"]);
}

Index globaltransport_getSampleOffset(MillisecondTime time) {
    return this->mstosamps(this->maximum(0, time - this->getEngine()->getCurrentTime()));
}

number globaltransport_getTempoAtSample(SampleIndex sampleOffset) {
    return (sampleOffset >= 0 && sampleOffset < this->vs ? this->globaltransport_tempo[(Index)sampleOffset] : this->globaltransport_lastTempo);
}

number globaltransport_getStateAtSample(SampleIndex sampleOffset) {
    return (sampleOffset >= 0 && sampleOffset < this->vs ? this->globaltransport_state[(Index)sampleOffset] : this->globaltransport_lastState);
}

number globaltransport_getState(MillisecondTime time) {
    return this->globaltransport_getStateAtSample(this->globaltransport_getSampleOffset(time));
}

number globaltransport_getBeatTime(MillisecondTime time) {
    number i = 2;

    while (i < this->globaltransport_beatTimeChanges->length && this->globaltransport_beatTimeChanges[(Index)(i + 1)] <= time) {
        i += 2;
    }

    i -= 2;
    number beatTimeBase = this->globaltransport_beatTimeChanges[(Index)i];

    if (this->globaltransport_getState(time) == 0)
        return beatTimeBase;

    number beatTimeBaseMsTime = this->globaltransport_beatTimeChanges[(Index)(i + 1)];
    number diff = time - beatTimeBaseMsTime;
    return beatTimeBase + this->mstobeats(diff);
}

bool globaltransport_setTempo(MillisecondTime time, number tempo, bool notify) {
    if ((bool)(notify)) {
        this->processTempoEvent(time, tempo);
        this->globaltransport_notify = true;
    } else {
        Index offset = (Index)(this->globaltransport_getSampleOffset(time));

        if (this->globaltransport_getTempoAtSample(offset) != tempo) {
            this->globaltransport_beatTimeChanges->push(this->globaltransport_getBeatTime(time));
            this->globaltransport_beatTimeChanges->push(time);
            fillSignal(this->globaltransport_tempo, this->vs, tempo, offset);
            this->globaltransport_lastTempo = tempo;
            this->globaltransport_tempoNeedsReset = true;
            return true;
        }
    }

    return false;
}

number globaltransport_getTempo(MillisecondTime time) {
    return this->globaltransport_getTempoAtSample(this->globaltransport_getSampleOffset(time));
}

bool globaltransport_setState(MillisecondTime time, number state, bool notify) {
    if ((bool)(notify)) {
        this->processTransportEvent(time, TransportState(state));
        this->globaltransport_notify = true;
    } else {
        Index offset = (Index)(this->globaltransport_getSampleOffset(time));

        if (this->globaltransport_getStateAtSample(offset) != state) {
            fillSignal(this->globaltransport_state, this->vs, state, offset);
            this->globaltransport_lastState = TransportState(state);
            this->globaltransport_stateNeedsReset = true;

            if (state == 0) {
                this->globaltransport_beatTimeChanges->push(this->globaltransport_getBeatTime(time));
                this->globaltransport_beatTimeChanges->push(time);
            }

            return true;
        }
    }

    return false;
}

bool globaltransport_setBeatTime(MillisecondTime time, number beattime, bool notify) {
    if ((bool)(notify)) {
        this->processBeatTimeEvent(time, beattime);
        this->globaltransport_notify = true;
        return false;
    } else {
        bool beatTimeHasChanged = false;
        float oldBeatTime = (float)(this->globaltransport_getBeatTime(time));
        float newBeatTime = (float)(beattime);

        if (oldBeatTime != newBeatTime) {
            beatTimeHasChanged = true;
        }

        this->globaltransport_beatTimeChanges->push(beattime);
        this->globaltransport_beatTimeChanges->push(time);
        return beatTimeHasChanged;
    }
}

number globaltransport_getBeatTimeAtSample(SampleIndex sampleOffset) {
    auto msOffset = this->sampstoms(sampleOffset);
    return this->globaltransport_getBeatTime(this->getEngine()->getCurrentTime() + msOffset);
}

array<number, 2> globaltransport_getTimeSignature(MillisecondTime time) {
    number i = 3;

    while (i < this->globaltransport_timeSignatureChanges->length && this->globaltransport_timeSignatureChanges[(Index)(i + 2)] <= time) {
        i += 3;
    }

    i -= 3;

    return {
        this->globaltransport_timeSignatureChanges[(Index)i],
        this->globaltransport_timeSignatureChanges[(Index)(i + 1)]
    };
}

array<number, 2> globaltransport_getTimeSignatureAtSample(SampleIndex sampleOffset) {
    auto msOffset = this->sampstoms(sampleOffset);
    return this->globaltransport_getTimeSignature(this->getEngine()->getCurrentTime() + msOffset);
}

bool globaltransport_setTimeSignature(MillisecondTime time, number numerator, number denominator, bool notify) {
    if ((bool)(notify)) {
        this->processTimeSignatureEvent(time, (int)(numerator), (int)(denominator));
        this->globaltransport_notify = true;
    } else {
        array<number, 2> currentSig = this->globaltransport_getTimeSignature(time);

        if (currentSig[0] != numerator || currentSig[1] != denominator) {
            this->globaltransport_timeSignatureChanges->push(numerator);
            this->globaltransport_timeSignatureChanges->push(denominator);
            this->globaltransport_timeSignatureChanges->push(time);
            return true;
        }
    }

    return false;
}

void globaltransport_advance() {
    if ((bool)(this->globaltransport_tempoNeedsReset)) {
        fillSignal(this->globaltransport_tempo, this->vs, this->globaltransport_lastTempo);
        this->globaltransport_tempoNeedsReset = false;

        if ((bool)(this->globaltransport_notify)) {
            this->getEngine()->sendTempoEvent(this->globaltransport_lastTempo);
        }
    }

    if ((bool)(this->globaltransport_stateNeedsReset)) {
        fillSignal(this->globaltransport_state, this->vs, this->globaltransport_lastState);
        this->globaltransport_stateNeedsReset = false;

        if ((bool)(this->globaltransport_notify)) {
            this->getEngine()->sendTransportEvent(TransportState(this->globaltransport_lastState));
        }
    }

    if (this->globaltransport_beatTimeChanges->length > 2) {
        this->globaltransport_beatTimeChanges[0] = this->globaltransport_beatTimeChanges[(Index)(this->globaltransport_beatTimeChanges->length - 2)];
        this->globaltransport_beatTimeChanges[1] = this->globaltransport_beatTimeChanges[(Index)(this->globaltransport_beatTimeChanges->length - 1)];
        this->globaltransport_beatTimeChanges->length = 2;

        if ((bool)(this->globaltransport_notify)) {
            this->getEngine()->sendBeatTimeEvent(this->globaltransport_beatTimeChanges[0]);
        }
    }

    if (this->globaltransport_timeSignatureChanges->length > 3) {
        this->globaltransport_timeSignatureChanges[0] = this->globaltransport_timeSignatureChanges[(Index)(this->globaltransport_timeSignatureChanges->length - 3)];
        this->globaltransport_timeSignatureChanges[1] = this->globaltransport_timeSignatureChanges[(Index)(this->globaltransport_timeSignatureChanges->length - 2)];
        this->globaltransport_timeSignatureChanges[2] = this->globaltransport_timeSignatureChanges[(Index)(this->globaltransport_timeSignatureChanges->length - 1)];
        this->globaltransport_timeSignatureChanges->length = 3;

        if ((bool)(this->globaltransport_notify)) {
            this->getEngine()->sendTimeSignatureEvent(
                (int)(this->globaltransport_timeSignatureChanges[0]),
                (int)(this->globaltransport_timeSignatureChanges[1])
            );
        }
    }

    this->globaltransport_notify = false;
}

void globaltransport_dspsetup(bool force) {
    if ((bool)(this->globaltransport_setupDone) && (bool)(!(bool)(force)))
        return;

    fillSignal(this->globaltransport_tempo, this->vs, this->globaltransport_lastTempo);
    this->globaltransport_tempoNeedsReset = false;
    fillSignal(this->globaltransport_state, this->vs, this->globaltransport_lastState);
    this->globaltransport_stateNeedsReset = false;
    this->globaltransport_setupDone = true;
}

bool stackprotect_check() {
    this->stackprotect_count++;

    if (this->stackprotect_count > 128) {
        console->log("STACK OVERFLOW DETECTED - stopped processing branch !");
        return true;
    }

    return false;
}

void updateTime(MillisecondTime time) {
    this->_currentTime = time;
    this->sampleOffsetIntoNextAudioBuffer = (SampleIndex)(rnbo_fround(this->msToSamps(time - this->getEngine()->getCurrentTime(), this->sr)));

    if (this->sampleOffsetIntoNextAudioBuffer >= (SampleIndex)(this->vs))
        this->sampleOffsetIntoNextAudioBuffer = (SampleIndex)(this->vs) - 1;

    if (this->sampleOffsetIntoNextAudioBuffer < 0)
        this->sampleOffsetIntoNextAudioBuffer = 0;
}

void assign_defaults()
{
    delaytilde_01_delay = 0;
    dspexpr_01_in1 = 0;
    dspexpr_01_in2 = 0;
    dspexpr_01_in3 = 0.2;
    pan_tilde_01_spread = 0.5;
    pan_tilde_01_pos = 0;
    dspexpr_02_in1 = 0;
    dspexpr_02_in2 = 0.5;
    param_01_value = 1;
    mstosamps_tilde_01_ms = 0;
    param_02_value = 0.5;
    param_03_value = 0.2;
    delaytilde_02_delay = 0;
    dspexpr_03_in1 = 0;
    dspexpr_03_in2 = 0;
    dspexpr_03_in3 = 0;
    pan_tilde_02_spread = 0.5;
    pan_tilde_02_pos = 0;
    dspexpr_04_in1 = 0;
    dspexpr_04_in2 = 0.5;
    param_04_value = 1;
    mstosamps_tilde_02_ms = 0;
    param_05_value = 0.5;
    _currentTime = 0;
    audioProcessSampleCount = 0;
    sampleOffsetIntoNextAudioBuffer = 0;
    zeroBuffer = nullptr;
    dummyBuffer = nullptr;
    signals[0] = nullptr;
    signals[1] = nullptr;
    signals[2] = nullptr;
    signals[3] = nullptr;
    didAllocateSignals = 0;
    vs = 0;
    maxvs = 0;
    sr = 44100;
    invsr = 0.00002267573696;
    delaytilde_01_lastDelay = -1;
    delaytilde_01_crossfadeDelay = 0;
    delaytilde_01_ramp = 0;
    delaytilde_01_rampInSamples = 0;
    delaytilde_01_del__maxdelay = 0;
    delaytilde_01_del_sizemode = 0;
    delaytilde_01_del_wrap = 0;
    delaytilde_01_del_reader = 0;
    delaytilde_01_del_writer = 0;
    delaytilde_01_setupDone = false;
    feedbacktilde_01_feedbackbuffer = nullptr;
    param_01_lastValue = 0;
    param_02_lastValue = 0;
    param_03_lastValue = 0;
    delaytilde_02_lastDelay = -1;
    delaytilde_02_crossfadeDelay = 0;
    delaytilde_02_ramp = 0;
    delaytilde_02_rampInSamples = 0;
    delaytilde_02_del__maxdelay = 0;
    delaytilde_02_del_sizemode = 0;
    delaytilde_02_del_wrap = 0;
    delaytilde_02_del_reader = 0;
    delaytilde_02_del_writer = 0;
    delaytilde_02_setupDone = false;
    feedbacktilde_02_feedbackbuffer = nullptr;
    param_04_lastValue = 0;
    param_05_lastValue = 0;
    globaltransport_tempo = nullptr;
    globaltransport_tempoNeedsReset = false;
    globaltransport_lastTempo = 120;
    globaltransport_state = nullptr;
    globaltransport_stateNeedsReset = false;
    globaltransport_lastState = 0;
    globaltransport_beatTimeChanges = { 0, 0 };
    globaltransport_timeSignatureChanges = { 4, 4, 0 };
    globaltransport_notify = false;
    globaltransport_setupDone = false;
    stackprotect_count = 0;
    _voiceIndex = 0;
    _noteNumber = 0;
    isMuted = 1;
}

// member variables

    number delaytilde_01_delay;
    number dspexpr_01_in1;
    number dspexpr_01_in2;
    number dspexpr_01_in3;
    number pan_tilde_01_spread;
    number pan_tilde_01_pos;
    number dspexpr_02_in1;
    number dspexpr_02_in2;
    number param_01_value;
    number mstosamps_tilde_01_ms;
    number param_02_value;
    number param_03_value;
    number delaytilde_02_delay;
    number dspexpr_03_in1;
    number dspexpr_03_in2;
    number dspexpr_03_in3;
    number pan_tilde_02_spread;
    number pan_tilde_02_pos;
    number dspexpr_04_in1;
    number dspexpr_04_in2;
    number param_04_value;
    number mstosamps_tilde_02_ms;
    number param_05_value;
    MillisecondTime _currentTime;
    SampleIndex audioProcessSampleCount;
    SampleIndex sampleOffsetIntoNextAudioBuffer;
    signal zeroBuffer;
    signal dummyBuffer;
    SampleValue * signals[4];
    bool didAllocateSignals;
    Index vs;
    Index maxvs;
    number sr;
    number invsr;
    number delaytilde_01_lastDelay;
    number delaytilde_01_crossfadeDelay;
    number delaytilde_01_ramp;
    long delaytilde_01_rampInSamples;
    Float64BufferRef delaytilde_01_del_buffer;
    Index delaytilde_01_del__maxdelay;
    Int delaytilde_01_del_sizemode;
    Index delaytilde_01_del_wrap;
    Int delaytilde_01_del_reader;
    Int delaytilde_01_del_writer;
    bool delaytilde_01_setupDone;
    signal feedbacktilde_01_feedbackbuffer;
    number param_01_lastValue;
    number param_02_lastValue;
    number param_03_lastValue;
    number delaytilde_02_lastDelay;
    number delaytilde_02_crossfadeDelay;
    number delaytilde_02_ramp;
    long delaytilde_02_rampInSamples;
    Float64BufferRef delaytilde_02_del_buffer;
    Index delaytilde_02_del__maxdelay;
    Int delaytilde_02_del_sizemode;
    Index delaytilde_02_del_wrap;
    Int delaytilde_02_del_reader;
    Int delaytilde_02_del_writer;
    bool delaytilde_02_setupDone;
    signal feedbacktilde_02_feedbackbuffer;
    number param_04_lastValue;
    number param_05_lastValue;
    signal globaltransport_tempo;
    bool globaltransport_tempoNeedsReset;
    number globaltransport_lastTempo;
    signal globaltransport_state;
    bool globaltransport_stateNeedsReset;
    number globaltransport_lastState;
    list globaltransport_beatTimeChanges;
    list globaltransport_timeSignatureChanges;
    bool globaltransport_notify;
    bool globaltransport_setupDone;
    number stackprotect_count;
    DataRef delaytilde_01_del_bufferobj;
    DataRef delaytilde_02_del_bufferobj;
    Index _voiceIndex;
    Int _noteNumber;
    Index isMuted;
    indexlist paramInitIndices;
    indexlist paramInitOrder;

};

PatcherInterface* creaternbomatic()
{
    return new rnbomatic();
}

#ifndef RNBO_NO_PATCHERFACTORY

extern "C" PatcherFactoryFunctionPtr GetPatcherFactoryFunction(PlatformInterface* platformInterface)
#else

extern "C" PatcherFactoryFunctionPtr rnbomaticFactoryFunction(PlatformInterface* platformInterface)
#endif

{
    Platform::set(platformInterface);
    return creaternbomatic;
}

} // end RNBO namespace

