/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   EC_Hydrax.cpp
 *  @brief  A photorealistic water plane component using Hydrax, http://www.ogre3d.org/tikiwiki/Hydrax
 */

#define OGRE_INTEROP

#include "DebugOperatorNew.h"

#include "EC_Hydrax.h"

#include "Scene.h"
#include "Framework.h"
#include "FrameAPI.h"
#include "AssetAPI.h"
#include "IAssetTransfer.h"
#include "IAsset.h"

#include "OgreWorld.h"
#include "Renderer.h"
#include "EC_Camera.h"
#include "Entity.h"
#include "OgreConversionUtils.h"

#include "AttributeMetadata.h"
#ifdef SKYX_ENABLED
#include "EC_SkyX.h"
#endif

#include <Hydrax.h>
#include <Noise/Perlin/Perlin.h>
#include <Noise/FFT/FFT.h>
#include <Modules/ProjectedGrid/ProjectedGrid.h>
#include <Modules/RadialGrid/RadialGrid.h>
#include <Modules/SimpleGrid/SimpleGrid.h>

#include "LoggingFunctions.h"
#include "MemoryLeakCheck.h"

struct EC_HydraxImpl
{
    EC_HydraxImpl() : hydrax(0), module(0) {}
    ~EC_HydraxImpl()
    {
        if (hydrax)
            hydrax->remove();
        SAFE_DELETE(hydrax);
        //SAFE_DELETE(module); ///< @todo Possible mem leak?
    }

    Hydrax::Hydrax *hydrax;
    Hydrax::Module::Module *module;
};

EC_Hydrax::EC_Hydrax(Scene* scene) :
    IComponent(scene),
    configRef(this, "Config ref", AssetReference("HydraxDemo.hdx")),
    visible(this, "Visible", true),
    position(this, "Position"),
//    noiseModule(this, "Noise module", 0),
//    noiseType(this, "Noise type", 0),
//    normalMode(this, "Normal mode", 0),
    impl(0)
{
/*
    static AttributeMetadata noiseTypeMetadata;
    static AttributeMetadata normalModeMetadata;
    static bool metadataInitialized = false;
    if (!metadataInitialized)
    {
        noiseTypeMetadata.enums[Perlin] = "Perlin";
        noiseTypeMetadata.enums[FFT] = "FFT";

        normalModeMetadata.enums[Texture] = "Texture";
        normalModeMetadata.enums[Vertex] = "Vertex";
        normalModeMetadata.enums[RTT] = "RTT";

        metadataInitialized = true;
    }

    noiseType.SetMetadata(&noiseTypeMetadata);
    normalMode.SetMetadata(&normalModeMetadata);
*/
    OgreWorldPtr w = scene->GetWorld<OgreWorld>();
    if (!w)
    {
        LogError("EC_SkyX: no OgreWorld available. Cannot be created.");
        return;
    }

    connect(w.get(), SIGNAL(ActiveCameraChanged(EC_Camera *)), SLOT(OnActiveCameraChanged(EC_Camera *)));
    connect(this, SIGNAL(ParentEntitySet()), SLOT(Create()));
}

EC_Hydrax::~EC_Hydrax()
{
    SAFE_DELETE(impl);
}

void EC_Hydrax::Create()
{
    SAFE_DELETE(impl);

    try
    {
        if (!ParentScene())
        {
            LogError("EC_Hydrax: no parent scene. Cannot be created.");
            return;
        }

        OgreWorldPtr w = ParentScene()->GetWorld<OgreWorld>();
        assert(w);

        if (!w->GetRenderer() || !w->GetRenderer()->GetActiveCamera())
            return; // Can't create Hydrax just yet, no main camera set.

        Ogre::Camera *cam = static_cast<EC_Camera *>(w->GetRenderer()->GetActiveCamera())->GetCamera();
        impl = new EC_HydraxImpl();
        impl->hydrax = new Hydrax::Hydrax(w->GetSceneManager(), cam, w->GetRenderer()->GetViewport());

        // Using projected grid module by default
        Hydrax::Module::ProjectedGrid *module = new Hydrax::Module::ProjectedGrid(impl->hydrax, new Hydrax::Noise::Perlin(),
            Ogre::Plane(Ogre::Vector3::UNIT_Y, Ogre::Vector3::ZERO), Hydrax::MaterialManager::NM_VERTEX);
        impl->hydrax->setModule(module);
        impl->module = module;

        // Load all parameters from config file
        impl->hydrax->loadCfg(configRef.Get().toStdString());

        position.Set(impl->hydrax->getPosition(), AttributeChange::Disconnected);

        impl->hydrax->create();

        connect(framework->Frame(), SIGNAL(PostFrameUpdate(float)), SLOT(Update(float)), Qt::UniqueConnection);
        connect(this, SIGNAL(AttributeChanged(IAttribute*, AttributeChange::Type)), SLOT(UpdateAttribute(IAttribute*)), Qt::UniqueConnection);
    }
    catch(Ogre::Exception &e)
    {
        // Currently if we try to create more than one Hydrax component we end up here due to Ogre internal name collision.
        LogError("Could not create EC_Hydrax: " + std::string(e.what()));
    }
}

void EC_Hydrax::OnActiveCameraChanged(EC_Camera *newActiveCamera)
{
    // If we haven't yet initialized, do a full init.
    if (!impl)
        Create();
    else // Otherwise, update the camera to an existing initialized Hydrax instance.
        if (impl && impl->hydrax)
            impl->hydrax->setCamera(newActiveCamera->GetCamera());
}

void EC_Hydrax::UpdateAttribute(IAttribute *attr)
{
    if (attr == &configRef)
    {
        QString assetRef = getconfigRef().ref;
        if (!assetRef.isEmpty())
        {
            AssetTransferPtr transfer = framework->Asset()->RequestAsset(getconfigRef().ref, "Binary");
            if (transfer.get())
                connect(transfer.get(), SIGNAL(Succeeded(AssetPtr)), SLOT(ConfigLoadSucceeded(AssetPtr)), Qt::UniqueConnection);
        }
        else
            LoadDefaultConfig();
    }

    if (!impl->hydrax)
        return;
    if (attr == &configRef)
    {
        impl->hydrax->loadCfg(configRef.Get().toStdString());
        // Config file can alter, position, update it accordinly
        position.Set(impl->hydrax->getPosition(), AttributeChange::Disconnected);
    }
    else if (attr == &visible)
        impl->hydrax->setVisible(visible.Get());
    else if (attr == &position)
        impl->hydrax->setPosition(position.Get());
/*
    else if (attr == &noiseModule || attr == &normalMode || &noiseType)
        UpdateNoiseModule();
*/
}
/*
void EC_Hydrax::UpdateNoiseModule()
{
    Hydrax::Noise::Noise *noise = 0;
    switch(noiseType.Get())
    {
    case Perlin:
        noise = new Hydrax::Noise::Perlin();
        break;
    case FFT:
        noise = new Hydrax::Noise::FFT();
        break;
    default:
        LogError("Invalid Hydrax noise module type.");
        return;
    }

    Hydrax::Module::Module *module = 0;
    switch(noiseModule.Get())
    {
    case ProjectedGrid:
        module = new Hydrax::Module::ProjectedGrid(impl->hydrax, noise, Ogre::Plane(Ogre::Vector3::UNIT_Y, Ogre::Vector3::ZERO),
            (Hydrax::MaterialManager::NormalMode)normalMode.Get());
        break;
    case RadialGrid:
        module = new Hydrax::Module::RadialGrid(impl->hydrax, noise, (Hydrax::MaterialManager::NormalMode)normalMode.Get());
        break;
    case SimpleGrid:
        module = new Hydrax::Module::SimpleGrid(impl->hydrax, noise, (Hydrax::MaterialManager::NormalMode)normalMode.Get());
        break;
    default:
        SAFE_DELETE(noise);
        LogError("Invalid Hydrax noise module type.");
        return;
    }
    //if (impl->module && noise)
    //    impl->module->setNoise(noise);

    impl->hydrax->setModule(module);
    impl->module = module;
    //impl->hydrax->loadCfg("HydraxDemo.hdx");
    impl->hydrax->create();
}
*/
void EC_Hydrax::Update(float frameTime)
{
    if (impl->hydrax)
    {
#ifdef SKYX_ENABLED
        ///\todo Store weak_ptr to EC_SkyX
        EntityList entities = ParentEntity()->ParentScene()->GetEntitiesWithComponent(EC_SkyX::TypeNameStatic());
        if (!entities.empty())
            impl->hydrax->setSunPosition((*entities.begin())->GetComponent<EC_SkyX>()->SunPosition());
#endif
        impl->hydrax->update(frameTime);
    }
}

void EC_Hydrax::ConfigLoadSucceeded(AssetPtr asset)
{
    if (!impl || !impl->hydrax || !impl->module)
    {
        LogError("EC_Hydrax: Could not apply loaded config, hydrax not initialized.");
        return;
    }

    std::vector<u8> rawData;
    asset->SerializeTo(rawData);
    QString configData = QString::fromAscii((const char*)&rawData[0], rawData.size());

    if (configData.isEmpty())
    {
        LogInfo("EC_Hydrax: Downloaded config is empty!");
        return;
    }

    try
    {
        // Update the noise module
        if (configData.contains("noise=fft", Qt::CaseInsensitive))
        {
            /// \note Using the FFT noise plugin seems to crash somewhere after we leave this function. 
            /// FFT looks better so would be nice to investigate further!
            if (impl->module->getNoise()->getName() != "FFT")
                impl->module->setNoise(new Hydrax::Noise::FFT());
        }
        else if (configData.contains("noise=perlin", Qt::CaseInsensitive))
        {
            if (impl->module->getNoise()->getName() != "Perlin")
                impl->module->setNoise(new Hydrax::Noise::Perlin());
        }
        else
        {
            LogError("EC_Hydrax: Unknown noise param in loaded config, acceptable = FFT/Perlin.");
            return;
        }

        // Load config from the asset data string.
        impl->hydrax->loadCfgString(configData.toStdString());
    }
    catch (Ogre::Exception &e)
    {
        LogError(std::string("EC_Hydrax: Ogre threw exception while loading new config: ") + e.what());
    }
}

void EC_Hydrax::LoadDefaultConfig()
{
    if (!impl || !impl->hydrax || !impl->module)
    {
        LogError("EC_Hydrax: Could not apply default config, hydrax not initialized.");
        return;
    }

    if (impl->module->getNoise()->getName() != "Perlin")
        impl->module->setNoise(new Hydrax::Noise::Perlin());

    // Load all parameters from the default config file we ship in /media/hydrax
    /// \todo Inspect if we can change the current SharedMode to HLSL or GLSL on the fly here, depending on the platform!
    impl->hydrax->loadCfg("HydraxDefault.hdx");
}
