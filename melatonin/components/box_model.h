#pragma once
#include "../helpers/colors.h"
#include "../helpers/component_helpers.h"
#include "collapsable_panel.h"
#include "melatonin_inspector/melatonin/component_model.h"

namespace melatonin
{
    class BoxModel : public juce::Component,
                     private juce::Label::Listener,
                     private ComponentModel::Listener
    {
    public:
        explicit BoxModel (ComponentModel& componentModel)
            : model (componentModel),
        tackTopToParentButton("tackTopToParentButton", juce::DrawableButton::ImageFitted),
        tackRightToParentButton("tackRightToParentButton", juce::DrawableButton::ImageFitted),
        tackBottomToParentButton("tackBottomToParentButton", juce::DrawableButton::ImageFitted),
        tackLeftToParentButton("tackLeftToParentButton", juce::DrawableButton::ImageFitted)
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

            juce::Label* parentLabels[4] = { &topToParentLabel, &rightToParentLabel, &bottomToParentLabel, &leftToParentLabel };
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

        void resized() override;

    private:
        void moreLayout();
        
        ComponentModel& model;

        juce::Label componentLabel;
        juce::Label parentComponentLabel;

        juce::Label widthLabel;
        juce::Label byLabel;
        juce::Label heightLabel;

        juce::Label topToParentLabel;
        juce::DrawableButton tackTopToParentButton;
        juce::Label rightToParentLabel;
        juce::DrawableButton tackRightToParentButton;
        juce::Label bottomToParentLabel;
        juce::DrawableButton tackBottomToParentButton;
        juce::Label leftToParentLabel;
        juce::DrawableButton tackLeftToParentButton;
        
        juce::Label paddingTopLabel,
            paddingRightLabel,
            paddingBottomLabel,
            paddingLeftLabel;

        int padding = 34;
        int paddingToParent = 44;
        juce::Path parentRectanglePath; // complicated b/c it's dashed
        bool isPaddingComponent { false };

        void handleAlignButtonStateChange( juce::Label &label, const juce::Identifier &direction, bool state );
        void initButton( juce::DrawableButton &button, juce::Label &label, const juce::Identifier &direction, const std::string &svgFile );
        
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
            if (changedLabel == &topToParentLabel || changedLabel == &bottomToParentLabel
                || changedLabel == &leftToParentLabel || changedLabel == &rightToParentLabel)
            {
                auto topVal = topToParentLabel.getText().getIntValue();
                auto leftVal = leftToParentLabel.getText().getIntValue();
                auto bottomVal = bottomToParentLabel.getText().getIntValue();
                auto rightVal = rightToParentLabel.getText().getIntValue();
                model.getSelectedComponent()->setTopLeftPosition (leftVal, topVal);
                model.getSelectedComponent()->setSize (model.getSelectedComponent()->getParentWidth() - rightVal - leftVal,
                    model.getSelectedComponent()->getParentHeight() - bottomVal - topVal);
            }
        }

        void componentModelChanged (ComponentModel&) override;

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

            topToParentLabel.setText (juce::String (boundsInParent.getY()), juce::dontSendNotification);
            topToParentLabel.setEditable (true);

            rightToParentLabel.setText (juce::String (model.getSelectedComponent()->getParentWidth() - model.getSelectedComponent()->getWidth() - boundsInParent.getX()), juce::dontSendNotification);
            rightToParentLabel.setEditable (true);

            bottomToParentLabel.setText (juce::String (model.getSelectedComponent()->getParentHeight() - model.getSelectedComponent()->getHeight() - boundsInParent.getY()), juce::dontSendNotification);
            bottomToParentLabel.setEditable (true);

            leftToParentLabel.setText (juce::String (boundsInParent.getX()), juce::dontSendNotification);
            leftToParentLabel.setEditable (true);

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
            juce::Label* labels[6] = { &widthLabel, &heightLabel, &topToParentLabel, &rightToParentLabel, &bottomToParentLabel, &leftToParentLabel };

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

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BoxModel)
    };
}
