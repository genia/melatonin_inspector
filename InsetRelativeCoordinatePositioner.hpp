//
//  InsetRelativeCoordinatePositioner.hpp
//  orandj - App
//
//  Created by GZ on 12/14/24.
//  Copyright © 2024 JUCE. All rights reserved.
//

#ifndef InsetRelativeCoordinatePositioner_hpp
#define InsetRelativeCoordinatePositioner_hpp

#include <JuceHeader.h>

using namespace std;
using namespace juce;

namespace melatonin {

class InsetRelativeCoordinatePositioner :
public Component::Positioner,
public ComponentListener
{
public:
    InsetRelativeCoordinatePositioner(Component& component, bool shouldFit = false ) : Component::Positioner(component)
    {
        fitToParent( shouldFit );
        component.addComponentListener (this);
    }
    
    static InsetRelativeCoordinatePositioner *makeForComponent(Component &component, bool shouldFit = false)
    {
        if (component.getPositioner() == nullptr)
        {
            InsetRelativeCoordinatePositioner *mrp = new InsetRelativeCoordinatePositioner( component, shouldFit );
            component.setPositioner( mrp );
        }
        return dynamic_cast<InsetRelativeCoordinatePositioner *>(component.getPositioner());
    }
    
    void fitToParent( bool shouldFit )
    {
        if (shouldFit)
        {
            params["rightToParent"] = 0;
            params["rightInsetEnabled"] = true;
            params["leftToParent"] = 0;
            params["leftInsetEnabled"] = true;
            params["topToParent"] = 0;
            params["topInsetEnabled"] = true;
            params["bottomToParent"] = 0;
            params["bottomInsetEnabled"] = true;
        }
        else
        {
            params["rightInsetEnabled"] = false;
            params["leftInsetEnabled"] = false;
            params["topInsetEnabled"] = false;
            params["bottomInsetEnabled"] = false;
        }
    }
    
    void applyNewBounds (const Rectangle<int>& newBounds) override
    {
        //    cout << getComponent().getName() << " new bounds: " <<  newBounds.toString() << endl;
        getComponent().setBounds( newBounds );
        grabPositionInParent();
    }
    
    void grabPositionInParent()
    {
        Component *parent = getComponent().getParentComponent();
        if (parent != nullptr)
        {
            params["rightToParent"] = parent->getWidth() - getComponent().getRight();
            params["bottomToParent"] = parent->getHeight() - getComponent().getBottom();
        }
    }
    
    void componentParentHierarchyChanged (Component& component) override
    {
        grabPositionInParent();
    }
    
    // since this is only called for the *parent*, both the positionable and its parent
    // must have a InsetRelativeCoordinatePositioner attached
    void componentMovedOrResized (Component &p, bool wasMoved, bool wasResized) override
    {
        int nChildren = p.getNumChildComponents();
        
        for (int i = 0; i < nChildren; i++)
        {
            Component *childComp = p.getChildComponent(i);
            Positioner *pos = childComp->getPositioner();
            InsetRelativeCoordinatePositioner *mrp = dynamic_cast<InsetRelativeCoordinatePositioner*>(pos);
            if (mrp != nullptr)
            {
                Rectangle<int> childBounds = childComp->getBounds();
                if (mrp->params["rightInsetEnabled"].getValue())
                {
                    int pWidth = p.getWidth();
                    int newRight = pWidth - (int)mrp->params["rightToParent"].getValue();
                    if (mrp->params["leftInsetEnabled"].getValue())
                    {
                        childBounds.setRight( newRight );
                    }
                    else
                    {
                        childBounds = childBounds.withRightX( newRight );
                    }// if it's just left, don't do anything
                }
                if (mrp->params["bottomInsetEnabled"].getValue())
                {
                    int pHeight = p.getHeight();
                    int newBottom = pHeight - (int)mrp->params["bottomToParent"].getValue();
                    if (mrp->params["topInsetEnabled"].getValue())
                    {
                        childBounds.setBottom( newBottom );
                    }
                    else
                    {
                        childBounds = childBounds.withBottomY( newBottom );
                    }// if it's just top, don't do anything
                }
                childComp->setBounds(childBounds);
            }
        }
    }
    
    std::map<juce::Identifier, juce::Value> params;
};

}

#endif /* InsetRelativeCoordinatePositioner_hpp */


