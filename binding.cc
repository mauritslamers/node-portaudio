#include "portaudiocpp/PortAudioCpp.hxx"
#include <iostream>
#include <node.h>
#include <assert.h>
#include <stdlib.h>

using namespace v8;
using namespace node;

// event names
//static Persistent<String> data_symbol;

const double SAMPLE_RATE = 44100.0;
const int FRAMES_PER_BUFFER = 64;

class PortAudio {
public:

  Handle<Function> registeredCallback_;
  
  static void Initialize (v8::Handle<v8::Object> target){
    HandleScope scope;
    
    Local<FunctionTemplate> t = FunctionTemplate::New(New);
    
    //t->Inherit(EventEmitter::constructor_template);
    t->InstanceTemplate()->SetInternalFieldCount(1);
    
    //data_symbol = NODE_PSYMBOL("data");
    
    NODE_SET_PROTOTYPE_METHOD(t, "start", Start);
    NODE_SET_PROTOTYPE_METHOD(t, "stop", Stop);
    NODE_SET_PROTOTYPE_METHOD(t, "setCallback", SetCallback);
    NODE_SET_PROTOTYPE_METHOD(t, "removeCallback", RemoveCallback);
    
    portaudio::DirectionSpecificStreamParameters outParams(sys.defaultOutputDevice(), 2, portaudio::FLOAT32, false, sys.defaultOutputDevice().defaultLowOutputLatency(),NULL);
    portaudio::StreamParameters params(portaudio::DirectionSpecificStreamParameters::null(), outParams, SAMPLE_RATE, FRAMES_PER_BUFFER, paClipOff);
    stream(params,this,&PortAudio::paCallback);  
    
  }
  
  // 
  int paCallback(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags){
      assert(outputBuffer != NULL);
      
      float **out = static_cast<float **>(outputBuffer);
      //fill stuff, do callbacks? 
      Handle<Array> samples ;
      bool useSamples = false;
      bool isMono = false;
      float sample;
      
      Local<Value> left;
      Local<Value> right; 
      
      if(registeredCallback_ != NULL){
        // call the function with the number of frames
        Local<Array> args = Array::New(1);

        args->Set(Integer::New(0),Integer::New(framesPerBuffer));
        samples = registeredCallback_->Call(this,1,args);
        left = samples->Get(Integer::New(0));
        if(left->isArray()){
          right = samples->Get(Integer::New(1));
        }
        else isMono = true;
        useSamples = true;
      }
      
      if(useSamples){
        for (unsigned int i = 0; i< framesPerBuffer; i++){
          sample = left->Get(Integer::New(i))->toFloat()->Value();
          out[0][i] = sample;
          if(isMono) out[1][i] = sample;
          else out[1][i] = right->Get(Integer::New(i))->toFloat()->Value();
        }
      }
      else {
        for (unsigned int i = 0; i< framesPerBuffer; ++i){
          out[0][i] = 0.0;
          out[1][i] = 0.0;
        }        
      }
      
      return paContinue;
    }

    bool Start() {
      if(running_) return false;
      
      /* Start the port audio callback */
      
      running_ = true;
      stream.start();
      return true;
    }
    
    bool Stop() {
      if(!running_) return false;
      
      /* Stop the port audio callback */
      
      stream.stop();
      running_ = false;
      
      return true;
    }
    
    bool setCallback(const Arguments& args){
      
      HandleScope scope;
      
      PortAudio *pa = ObjectWrap::Unwrap<PortAudio>(args.This());
      
      if(pa.registeredCallback_ == NULL) pa.registeredCallback_ = NULL; // overwrite
      
      if (args.Length() == 0 || !args[0]->IsFunction()){
        return ThrowException(Exception::Error(String::New("Must give a function to register")));
      }
      pa.registeredCallback_ = args[0];
      
    }
    
    bool removeCallback(const Arguments& args){
      HandleScope scope;
      
      PortAudio *pa = ObjectWrap::Unwrap<PortAudio>(args.This());
      
      pa.registeredCallback_ = NULL;
    }

protected:
  static Handle<Value> New(const Arguments& args){
    HandleScope scope;
    
    PortAudio *portaudio = new PortAudio();
    portaudio->Wrap(args.This());
    
    return args.This();
  }
  
  PortAudio () {
    portaudio::System &sys = portaudio::System::instance();
  }
  
  ~PortAudio () {
    stream.close();
    stream.terminate();
  }
  
private:
  portaudio::AutoSystem autoSys;
  portaudio::System sys;
  //portaudio::StreamParameters params;
  //portaudio::MemFunCallbackStream stream;
  portaudio::MemFunCallbackStream<PortAudio> stream;
  
  bool running_;
}