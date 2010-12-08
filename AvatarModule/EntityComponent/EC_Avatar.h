// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Avatar_EC_Avatar_h
#define incl_Avatar_EC_Avatar_h

#include "IComponent.h"
#include "RexUUID.h"
#include "AvatarModuleApi.h"
#include "Declare_EC.h"
#include "AssetFwd.h"

namespace Avatar
{
    class AvatarHandler;
    typedef boost::shared_ptr<AvatarHandler> AvatarHandlerPtr;
};

//! Avatar component.
/**
<table class="header">
<tr>
<td>
<h2>Avatar</h2>
This component generates the required Mesh, Placeable, AnimationController and AvatarAppearance
components to an entity to display an avatar. Not to be used in OpenSim worlds, where avatar generation
is handled in a hardcoded manner instead.

Registered by Avatar::AvatarModule.

<b>Attributes</b>:
<ul>
<li>QString: appearanceId
<div>Asset id for the avatar appearance file that will be used to generate the visible avatar.</div>
</ul>

<b>Exposes the following scriptable functions:</b>
<ul>
</ul>

<b>Reacts on the following actions:</b>
<ul>
</ul>

Does not emit any actions.

<b>Depends on the components Mesh, Placeable, AnimationController and AvatarAppearance</b>.
</table>
*/
class AV_MODULE_API EC_Avatar : public IComponent
{
    Q_OBJECT
    
    DECLARE_EC(EC_Avatar);
public:
    //! Asset id for the avatar appearance file that will be used to generate the visible avatar. Asset request is handled by the component.
    Q_PROPERTY(QString appearanceId READ getappearanceId WRITE setappearanceId);
    DEFINE_QPROPERTY_ATTRIBUTE(QString, appearanceId);

    //! Set component as serializable.
    virtual bool IsSerializable() const { return true; }
        
    //! Destructor
    virtual ~EC_Avatar();
    
private slots:
    //! Called when some of the attributes has been changed.
    void AttributeUpdated(IAttribute *attribute);
    
    void OnAvatarAppearanceLoaded(IAssetTransfer *transfer);
private:
    //! constructor
    /*! \param module avatar module
     */
    EC_Avatar(IModule* module);
        
    //! Category for Asset events
    event_category_id_t asset_event_category_;
    
    //! Avatar appearance file request tag
    request_tag_t appearance_tag_;
    
    //! Avatar handler, used to do the heavy lifting
    Avatar::AvatarHandlerPtr avatar_handler_;
};

#endif
