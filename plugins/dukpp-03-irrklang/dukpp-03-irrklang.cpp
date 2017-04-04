#include "dukpp-03-irrklang/dukpp-03-irrklang.h"

#include <db/dbtypename.h>

#include <cassert>


static bool _engineIsCurrentlyPlaying(::irrklang::ISoundEngine* e, const sad::String& s)
{
    return e->isCurrentlyPlaying(s.c_str());
}

static ::irrklang::ISound* _enginePlay2DString(::irrklang::ISoundEngine* e, const sad::String& s, bool playLooped, bool startPaused)
{
    return e->play2D(s.c_str(), playLooped, startPaused);
}

static ::irrklang::ISound* _enginePlay2DSource(::irrklang::ISoundEngine* e,  ::irrklang::ISoundSource* s, bool playLooped, bool startPaused)
{
    return e->play2D(s, playLooped, startPaused);
}


void sad::dukpp03irrklang::init(sad::dukpp03::Context* ctx)
{
    // Bindings for  sad::irrklang::Engine
    {
        sad::dukpp03::ClassBinding* c = new sad::dukpp03::ClassBinding();
        c->addMethod("tryLoad",  sad::dukpp03::bind_method::from(&sad::irrklang::Engine::tryLoad));
        c->addMethod("get",  sad::dukpp03::bind_method::from(&sad::irrklang::Engine::get));
        
        ctx->addClassBinding("sad::irrklang::Engine", c);

        ctx->registerCallable(
            "SadIrrKlangEngineRef", 
            sad::dukpp03::make_function::from(sad::irrklang::Engine::ref)
        );
        ctx->registerCallable(
            "SadIrrKlangEngineERef", 
            sad::dukpp03::make_function::from(sad::irrklang::Engine::eref)
        );

        bool result = ctx->eval("sad.irrklang = {}; sad.irrklang.Engine = {}; sad.irrklang.Engine.ref = SadIrrKlangEngineRef; sad.irrklang.Engine.eref = SadIrrKlangEngineERef;");    
        assert(result);
    }
    // Bindings for  ::irrklang::ISoundEngine
    {
        bool result = ctx->eval("_IrrKlangSoundEngine = function() {};");    
        assert(result);


        sad::dukpp03::ClassBinding* c = new sad::dukpp03::ClassBinding();
        bool (::irrklang::ISoundEngine::*isCurrentlyPlaying1)(::irrklang::ISoundSource*) = &::irrklang::ISoundEngine::isCurrentlyPlaying;

        c->setPrototypeFunction("_IrrKlangSoundEngine");
        c->addMethod("isCurrentlyPlayingSoundSource",  sad::dukpp03::bind_method::from(isCurrentlyPlaying1));
        c->addMethod("stopAllSounds",  sad::dukpp03::bind_method::from(&::irrklang::ISoundEngine::stopAllSounds));
        c->addMethod("setAllSoundsPaused",  sad::dukpp03::bind_method::from(&::irrklang::ISoundEngine::setAllSoundsPaused));
        
        ctx->addClassBinding("::irrklang::ISoundEngine", c);

        ctx->registerCallable(
            "_IrrKlangEngineIsCurrentlyPlaying", 
            sad::dukpp03::make_function::from(_engineIsCurrentlyPlaying)
        );

        ctx->registerCallable(
            "_IrrKlangEnginePlay2DString", 
            sad::dukpp03::make_function::from(_enginePlay2DString)
        );
        ctx->registerCallable(
            "_IrrKlangEnginePlay2DSource", 
            sad::dukpp03::make_function::from(_enginePlay2DSource)
        );
        

        result = ctx->eval("_IrrKlangSoundEngine.isCurrentlyPlaying= function(s) { if (typeof s == \"string\") return _IrrKlangEngineIsCurrentlyPlaying(this, s); else return  this.isCurrentlyPlayingSoundSource(s);   };  ");
        assert(result);

        result = ctx->eval(
            "_IrrKlangSoundEngine.play2D = function(s, playLooped, startPaused) {"
                "var _p  = (typeof playLooped == \"undefined\") ? false : playLooped;"
                "var _sp = (typeof startPaused == \"undefined\") ? false : startPaused;"
                "if (typeof s == \"string\") return _IrrKlangEnginePlay2DString(this, s, _p, _sp); "
                                       "else return _IrrKlangEnginePlay2DSource(this, s, _p, _sp);"
            "};  "
        );
        assert(result);

    }
}

DECLARE_COMMON_TYPE(::irrklang::ISoundSource)
DECLARE_COMMON_TYPE(::irrklang::ISoundEngine)
DECLARE_COMMON_TYPE(::irrklang::ISound)
