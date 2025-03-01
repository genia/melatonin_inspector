#pragma once
#include "../helpers/colors.h"
#include "../helpers/component_helpers.h"
#include "collapsable_panel.h"
#include "melatonin_inspector/melatonin/component_model.h"
#include "melatonin_inspector/InsetRelativeCoordinatePositioner.hpp"

namespace melatonin
{
    class BoxModel : public juce::Component,
                     private juce::Label::Listener,
                     private ComponentModel::Listener
    {
    public:
        explicit BoxModel (ComponentModel& componentModel)
            : model (componentModel),
        insetTopButton("insetTopButton", juce::DrawableButton::ImageFitted),
        insetRightButton("insetRightButton", juce::DrawableButton::ImageFitted),
        insetBottomButton("insetBottomButton", juce::DrawableButton::ImageFitted),
        insetLeftButton("insetLeftButton", juce::DrawableButton::ImageFitted)
        {
            addAndMakeVisible (componentLabel);
            componentLabel.setColour (juce::Label::textColourId, colors::boxModelBoundingBox);
            componentLabel.setJustificationType (juce::Justification::centredLeft);

            addAndMakeVisible (parentComponentLabel);
            parentComponentLabel.setJustificationType (juce::Justification::centredLeft);

            addAndMakeVisible (widthLabel);
            widthLabel.addListener (this);
           #if JUCE_MAJOR_VERSION == 8
            widthLabel.setFont (juce::FontOptions (20.0f));
           #else
            widthLabel.setFont (20.0f);
           #endif
            widthLabel.setJustificationType (juce::Justification::centredRight);
            widthLabel.setColour (juce::Label::textColourId, colors::white);

            addAndMakeVisible (byLabel);
            byLabel.setText (juce::CharPointer_UTF8("\xc3\x97"), juce::dontSendNotification);
           #if JUCE_MAJOR_VERSION == 8
            byLabel.setFont (juce::FontOptions (20.f));
           #else
            byLabel.setFont (20.f);
           #endif
            byLabel.setJustificationType (juce::Justification::centred);
            byLabel.setColour (juce::Label::textColourId, colors::white);

            addAndMakeVisible (heightLabel);
            heightLabel.addListener (this);
           #if JUCE_MAJOR_VERSION == 8
            heightLabel.setFont (juce::FontOptions (20.f));
           #else
            heightLabel.setFont (20.f);
           #endif
            heightLabel.setJustificationType (juce::Justification::centredLeft);
            heightLabel.setColour (juce::Label::textColourId, colors::white);

            juce::Label* parentLabels[4] = { &topInsetLabel, &rightInsetLabel, &bottomInsetLabel, &leftInsetLabel };
            juce::Label* paddingLabels[4] = { &paddingTopLabel, &paddingRightLabel, &paddingLeftLabel, &paddingBottomLabel };

            for (auto parentLabel : parentLabels)
            {
                addAndMakeVisible (parentLabel);
                parentLabel->setText ("-", juce::dontSendNotification);
                parentLabel->setJustificationType (juce::Justification::centred);
                parentLabel->addListener (this);

                // centers juce::TextEditor (hack since juce::Label is not doing it by default)
                parentLabel->onEditorShow = [parentLabel] {
                    if (auto editor = parentLabel->getCurrentTextEditor())
                    {
                        auto labelJustification = parentLabel->getJustificationType();
                        if (editor->getJustificationType() != labelJustification)
                        {
                            editor->setJustification (parentLabel->getJustificationType());
                        }
                    }
                };
            }

            for (auto l : paddingLabels)
            {
                addChildComponent (l);
                l->setText ("-", juce::dontSendNotification);
                l->setJustificationType (juce::Justification::centred);
                l->setColour (juce::TextEditor::ColourIds::highlightColourId, colors::boxModelBoundingBox.darker());

                // centers juce::TextEditor (hack since juce::Label is not doing it by default)
                l->onEditorShow = [l] {
                    if (auto editor = l->getCurrentTextEditor())
                    {
                        auto labelJustification = l->getJustificationType();
                        if (editor->getJustificationType() != labelJustification)
                        {
                            editor->setJustification (l->getJustificationType());
                        }
                    }
                };

                l->onEditorHide = [l] {
                    auto text = l->getText (true);
                    if (text.getIntValue() == 0)
                        l->setText ("0", juce::dontSendNotification);
                };
            }
            moreLayout();
            
            model.addListener (*this);
        }

        ~BoxModel() override
        {
            model.removeListener (*this);
        }

        void paint (juce::Graphics& g) override
        {
            // dashed line rectangles be hard, yo!
            g.setColour (colors::label.darker (0.6f));
            float dashLengths[2] = { 2.f, 2.f };
            parentRectanglePath.clear();
            parentRectanglePath.addRectangle (parentComponentRectangle());
            auto parentStroke = juce::PathStrokeType (0.5);
            parentStroke.createDashedStroke (parentRectanglePath, parentRectanglePath, dashLengths, 2);
            g.strokePath (parentRectanglePath, parentStroke);

            g.setColour (colors::boxModelBoundingBox);
            g.drawRect (componentRectangle(), 1);

            // draw padding
            g.setColour (colors::boxModelBoundingBox.withAlpha (0.2f));
            g.drawRect (componentRectangle(), (int) padding);
        }

    private:
        
        ComponentModel& model;

        juce::Label componentLabel;
        juce::Label parentComponentLabel;

        juce::Label widthLabel;
        juce::Label byLabel;
        juce::Label heightLabel;

        juce::Label topInsetLabel;
        juce::DrawableButton insetTopButton;
        juce::Label rightInsetLabel;
        juce::DrawableButton insetRightButton;
        juce::Label bottomInsetLabel;
        juce::DrawableButton insetBottomButton;
        juce::Label leftInsetLabel;
        juce::DrawableButton insetLeftButton;
        
        juce::Label paddingTopLabel,
            paddingRightLabel,
            paddingBottomLabel,
            paddingLeftLabel;

        int padding = 34;
        int paddingToParent = 44;
        juce::Path parentRectanglePath; // complicated b/c it's dashed
        bool isPaddingComponent { false };
        
        void labelTextChanged (juce::Label* changedLabel) override
        {

            if (changedLabel == &widthLabel || changedLabel == &heightLabel)
            {
                model.getSelectedComponent()->setSize (widthLabel.getText().getIntValue(), heightLabel.getText().getIntValue());
            }
            if (changedLabel == &paddingRightLabel || changedLabel == &paddingLeftLabel
                || changedLabel == &paddingTopLabel || changedLabel == &paddingBottomLabel)
            {
                updateDisplayedComponentPaddingProperties (paddingRightLabel.getText().getIntValue(), paddingLeftLabel.getText().getIntValue(), paddingTopLabel.getText().getIntValue(), paddingBottomLabel.getText().getIntValue());
            }
            if (changedLabel == &topInsetLabel || changedLabel == &bottomInsetLabel
                || changedLabel == &leftInsetLabel || changedLabel == &rightInsetLabel)
            {
                auto topVal = topInsetLabel.getText().getIntValue();
                auto leftVal = leftInsetLabel.getText().getIntValue();
                auto bottomVal = bottomInsetLabel.getText().getIntValue();
                auto rightVal = rightInsetLabel.getText().getIntValue();
                model.getSelectedComponent()->setTopLeftPosition (leftVal, topVal);
                model.getSelectedComponent()->setSize (model.getSelectedComponent()->getParentWidth() - rightVal - leftVal,
                    model.getSelectedComponent()->getParentHeight() - bottomVal - topVal);
            }
        }

        juce::Rectangle<int> parentComponentRectangle()
        {
            return getLocalBounds().reduced (padding);
        }

        juce::Rectangle<int> componentRectangle()
        {
            return parentComponentRectangle().reduced (paddingToParent);
        }

        void updateLabels()
        {
            if (!model.getSelectedComponent())
            {
                reset();
                return;
            }
            auto boundsInParent = model.getSelectedComponent()->getBoundsInParent();

            parentComponentLabel.setText (componentString (model.getSelectedComponent()->getParentComponent()), juce::dontSendNotification);
            componentLabel.setText (componentString (model.getSelectedComponent()), juce::dontSendNotification);

            widthLabel.setText (juce::String (model.getSelectedComponent()->getWidth()), juce::dontSendNotification);
            heightLabel.setText (juce::String (model.getSelectedComponent()->getHeight()), juce::dontSendNotification);

            widthLabel.setEditable (true);
            heightLabel.setEditable (true);

            topInsetLabel.setText (juce::String (boundsInParent.getY()), juce::dontSendNotification);
            topInsetLabel.setEditable (true);

            rightInsetLabel.setText (juce::String (model.getSelectedComponent()->getParentWidth() - model.getSelectedComponent()->getWidth() - boundsInParent.getX()), juce::dontSendNotification);
            rightInsetLabel.setEditable (true);

            bottomInsetLabel.setText (juce::String (model.getSelectedComponent()->getParentHeight() - model.getSelectedComponent()->getHeight() - boundsInParent.getY()), juce::dontSendNotification);
            bottomInsetLabel.setEditable (true);

            leftInsetLabel.setText (juce::String (boundsInParent.getX()), juce::dontSendNotification);
            leftInsetLabel.setEditable (true);

            repaint();
        }

        // See Melatonin's PaddedComponent or store this info in your component's getProperties
        void updatePaddingLabelsIfNeeded()
        {
            if (!model.getSelectedComponent())
            {
                // if model.getSelectedComponent() is null, getting props will fail
                juce::Label* paddingLabels[4] = { &paddingTopLabel, &paddingRightLabel, &paddingLeftLabel, &paddingBottomLabel };

                for (auto pl : paddingLabels)
                {
                    pl->setText ("-", juce::dontSendNotification);
                    pl->setEditable (false);
                    pl->removeListener (this);
                }

                return;
            }
            auto component = model.getSelectedComponent();
            auto props = component->getProperties();
            auto hasTopPadding = props.contains ("paddingTop");
            auto hasBottomPadding = props.contains ("paddingBottom");
            auto hasLeftPadding = props.contains ("paddingLeft");
            auto hasRightPadding = props.contains ("paddingRight");

            int paddingTop = props["paddingTop"];
            int paddingBottom = props["paddingBottom"];
            int paddingLeft = props["paddingLeft"];
            int paddingRight = props["paddingRight"];

            isPaddingComponent = hasBottomPadding || hasTopPadding || hasLeftPadding || hasRightPadding;
            paddingTopLabel.setVisible (isPaddingComponent);
            paddingBottomLabel.setVisible (isPaddingComponent);
            paddingLeftLabel.setVisible (isPaddingComponent);
            paddingRightLabel.setVisible (isPaddingComponent);

            paddingTopLabel.setText (hasTopPadding ? juce::String (paddingTop) : "-", juce::dontSendNotification);
            paddingTopLabel.setEditable (hasTopPadding);
            paddingTopLabel.addListener (this);

            paddingBottomLabel.setText (hasBottomPadding ? juce::String (paddingBottom) : "-", juce::dontSendNotification);
            paddingBottomLabel.setEditable (hasBottomPadding);
            paddingBottomLabel.addListener (this);

            paddingLeftLabel.setText (hasLeftPadding ? juce::String (paddingLeft) : "-", juce::dontSendNotification);
            paddingLeftLabel.setEditable (hasLeftPadding);
            paddingLeftLabel.addListener (this);

            paddingRightLabel.setText (hasRightPadding ? juce::String (paddingRight) : "-", juce::dontSendNotification);
            paddingRightLabel.setEditable (hasRightPadding);
            paddingRightLabel.addListener (this);
        }

        void updateDisplayedComponentPaddingProperties (double paddingRight, double paddingLeft, double paddingTop, double paddingBottom)
        {
            if (model.getSelectedComponent())
            {
                auto& props = model.getSelectedComponent()->getProperties();
                props.set ("paddingLeft", paddingLeft);
                props.set ("paddingTop", paddingTop);
                props.set ("paddingRight", paddingRight);
                props.set ("paddingBottom", paddingBottom);
                model.getSelectedComponent()->resized();
                model.getSelectedComponent()->repaint();
            }
        }

        void reset()
        {
            juce::Label* labels[6] = { &widthLabel, &heightLabel, &topInsetLabel, &rightInsetLabel, &bottomInsetLabel, &leftInsetLabel };

            for (auto label : labels)
            {
                label->setText ("-", juce::dontSendNotification);
                label->setEditable (false);
            }

            juce::Label* paddingLabels[4] = { &paddingTopLabel, &paddingRightLabel, &paddingLeftLabel, &paddingBottomLabel };
            for (auto label : paddingLabels)
            {
                label->setVisible (false);
            }

            componentLabel.setText ("", juce::dontSendNotification);
            parentComponentLabel.setText ("", juce::dontSendNotification);
        }
        
        void handleAlignButtonStateChange( juce::Label &label, const juce::Identifier &property, bool state )
        {
            Component *component = model.getSelectedComponent();
            if (component == nullptr)
                return;
            InsetRelativeCoordinatePositioner *mrp = dynamic_cast<InsetRelativeCoordinatePositioner*>( component->getPositioner() );
            if (mrp == nullptr)
                return;

            mrp->params[property] = state;
        }

        void
        initButton( juce::DrawableButton &button, juce::Label &label, const juce::Identifier &property, const String &svgFile )
        {
            int size = 0;
            auto rsrcName = svgFile.replace(".", "_");
            auto data = InspectorBinaryData::getNamedResource (rsrcName.toUTF8(), size);
            std::unique_ptr< Drawable > drawable = Drawable::createFromImageData(data, size);

            Colour origColour( 0xff5f6368 );
            juce::Colour newColour( Colours::lightgrey );
            std::unique_ptr< Drawable > one = drawable->createCopy();
            one->replaceColour(origColour, newColour);
            std::unique_ptr< Drawable > two = drawable->createCopy();
            two->replaceColour(origColour, newColour.darker());
            std::unique_ptr< Drawable > three = drawable->createCopy();
            three->replaceColour(origColour, newColour.darker(.8));
            
            juce::Colour newColourOn( Colours::red );
            std::unique_ptr< Drawable > oneOn = drawable->createCopy();
            oneOn->replaceColour(origColour, newColourOn);
            std::unique_ptr< Drawable > twoOn = drawable->createCopy();
            twoOn->replaceColour(origColour, newColourOn.darker());
            std::unique_ptr< Drawable > threeOn = drawable->createCopy();
            threeOn->replaceColour(origColour, newColourOn.darker(.8));
            
            auto buttonSize = 20;
            button.setSize(buttonSize, buttonSize);
            button.setToggleable( true );
            button.setClickingTogglesState( true );
            button.setImages(one.get(), two.get(), three.get(), nullptr,
                             oneOn.get(), twoOn.get(), threeOn.get(), nullptr );
            
            button.onClick = [this, property, &label, &button] {
                bool state = button.getToggleState();
                this->handleAlignButtonStateChange( label, property, state );
            };
            
        }

        void moreLayout()
        {
            const std::array<std::tuple<juce::DrawableButton &, juce::Label &, juce::Identifier, juce::String>, 4> buttonDescArray = {{
                {insetTopButton, topInsetLabel, "topInsetEnabled", "insetTop.svg", },
                {insetRightButton, rightInsetLabel, "rightInsetEnabled", "insetRight.svg" },
                {insetBottomButton, bottomInsetLabel, "bottomInsetEnabled", "insetBottom.svg", },
                {insetLeftButton, topInsetLabel, "leftInsetEnabled", "insetLeft.svg"}
            }};

            for (auto [ button, label, property, filename ] : buttonDescArray)
            {
                initButton( button, label, property, filename );
                addAndMakeVisible( button );
            }
        }

        void resized() override
        {
            auto bounds = parentComponentRectangle();
            auto center = bounds.getCentre();
            auto labelHeight = 30;
            auto buttonDistance = 20;
            
            parentComponentLabel.setBounds (bounds.getX(), bounds.getY() - labelHeight + 4, bounds.getWidth(), labelHeight);
            componentLabel.setBounds (componentRectangle().getX(), componentRectangle().getY() - labelHeight + 4, componentRectangle().getWidth(), labelHeight);
            
            widthLabel.setBounds (center.getX() - 10 - paddingToParent, center.getY() - 15, paddingToParent, labelHeight);
            byLabel.setBounds (center.getX() - 10, center.getY() - 15, 20, labelHeight);
            heightLabel.setBounds (center.getX() + 10, center.getY() - 15, paddingToParent, labelHeight);
            
            topInsetLabel.setBounds (center.getX() - paddingToParent / 2, padding + paddingToParent / 2 - labelHeight / 2 - 3, paddingToParent, labelHeight);
            insetTopButton.setTopLeftPosition(topInsetLabel.getX()-buttonDistance, topInsetLabel.getY() + topInsetLabel.getHeight()/4);
            
            rightInsetLabel.setBounds (getWidth() - padding - paddingToParent / 2 - paddingToParent / 2, center.getY() - labelHeight / 2, paddingToParent, labelHeight);
            insetRightButton.setTopLeftPosition(rightInsetLabel.getX() + (rightInsetLabel.getWidth() - insetRightButton.getWidth())/2, rightInsetLabel.getY() - buttonDistance);
            
            bottomInsetLabel.setBounds (center.getX() - paddingToParent / 2, getHeight() - padding - paddingToParent / 2 - labelHeight / 2 + 3, paddingToParent, labelHeight);
            insetBottomButton.setTopLeftPosition(bottomInsetLabel.getX()-buttonDistance, bottomInsetLabel.getY() + bottomInsetLabel.getHeight()/4);
            
            leftInsetLabel.setBounds (padding + paddingToParent / 2 - paddingToParent / 2, center.getY() - labelHeight / 2, paddingToParent, labelHeight);
            insetLeftButton.setTopLeftPosition(leftInsetLabel.getX() + (leftInsetLabel.getWidth() - insetLeftButton.getWidth())/2, leftInsetLabel.getY() - buttonDistance);
            
            auto area1 = bounds.reduced (paddingToParent)
                .removeFromTop (padding)
                .withSizeKeepingCentre (padding, padding);
            paddingTopLabel.setBounds (area1);
            
            auto area2 = bounds.reduced (paddingToParent)
                .removeFromBottom (padding)
                .withSizeKeepingCentre (padding, padding);
            paddingBottomLabel.setBounds (area2);
            
            auto area3 = bounds.reduced (paddingToParent)
                .removeFromLeft (padding)
                .withSizeKeepingCentre (padding, padding);
            paddingLeftLabel.setBounds (area3);
            
            auto area4 = bounds.reduced (paddingToParent)
                .removeFromRight (padding)
                .withTrimmedTop (padding)
                .withTrimmedBottom (padding)
                .withSizeKeepingCentre (padding, padding);
            paddingRightLabel.setBounds (area4);
        }

        void componentModelChanged (ComponentModel&) override
        {
            updateLabels();
            updatePaddingLabelsIfNeeded();
            
            Component *comp = model.getSelectedComponent();
            if (comp == nullptr)
                return;
            InsetRelativeCoordinatePositioner *mrp = dynamic_cast<InsetRelativeCoordinatePositioner*>( comp->getPositioner() );
            if (mrp != nullptr)
            {
                insetRightButton.setEnabled(true);
                insetRightButton.setToggleState(mrp->params["rightInsetEnabled"].getValue(), juce::dontSendNotification);
                insetLeftButton.setEnabled(true);
                insetLeftButton.setToggleState(mrp->params["leftInsetEnabled"].getValue(), juce::dontSendNotification);
                insetTopButton.setEnabled(true);
                insetTopButton.setToggleState(mrp->params["topInsetEnabled"].getValue(), juce::dontSendNotification);
                insetBottomButton.setEnabled(true);
                insetBottomButton.setToggleState(mrp->params["bottomInsetEnabled"].getValue(), juce::dontSendNotification);
            }
            else
            {
                insetRightButton.setEnabled(false);
                insetRightButton.setToggleState(false, juce::dontSendNotification);
                insetLeftButton.setEnabled(false);
                insetLeftButton.setToggleState(false, juce::dontSendNotification);
                insetTopButton.setEnabled(false);
                insetTopButton.setToggleState(false, juce::dontSendNotification);
                insetBottomButton.setEnabled(false);
                insetBottomButton.setToggleState(false, juce::dontSendNotification);
            }
        }


        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BoxModel)
    };
}
