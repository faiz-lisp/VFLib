// Copyright (C) 2008 by One Guy Group, Inc., All rights reserved worldwide.

namespace Ui {

namespace Model {

const String Button::getName ()
{
  return String::empty;
}

bool Button::isMomentary ()
{
  return false;
}

void Button::clicked (const ModifierKeys&)
{
}

void Button::pressed ()
{
}

void Button::released ()
{
}

//------------------------------------------------------------------------------

CheckButton::CheckButton ()
  : m_checked (false)
{
}

bool CheckButton::isChecked ()
{
  return m_checked;;
}

void CheckButton::setChecked (bool checked)
{
  if (m_checked != checked)
  {
    m_checked = checked;
    
    //updateAllViews ();
    modelChanged ();
    m_listeners.call (&Base::Listener::onModelChanged, (Model::Base*)this);
    m_listeners.call (&Listener::onCheckButtonChecked, this);
  }
}

//------------------------------------------------------------------------------

DummyButton::DummyButton (bool enabled)
{
  setEnabled (enabled);
}

//------------------------------------------------------------------------------

DummyCheckButton::DummyCheckButton (bool enabled)
{
  setEnabled (enabled);
}

void DummyCheckButton::clicked (ModifierKeys const&)
{
  setChecked (!isChecked ());
}

}

//------------------------------------------------------------------------------

namespace Facade {

Model::Button& Button::getModel ()
{
  return dynamic_cast <Model::Button&> (Base::getModel());
}

Control::Button& Button::getControl ()
{
  return dynamic_cast <Control::Button&> (Base::getControl ());
}

void Button::setButtonState (bool isMouseOverButton, bool isButtonDown)
{
  m_isMouseOverButton = isMouseOverButton;
  m_isButtonDown = isButtonDown;
}

bool Button::isMouseOverButton()
{
  return m_isMouseOverButton;
}

bool Button::isButtonDown()
{
  return m_isButtonDown;
}

void Button::onAttach ()
{
  Base::onAttach ();
}

//------------------------------------------------------------------------------

Model::CheckButton& CheckButton::getModel ()
{
  return dynamic_cast <Model::CheckButton&> (Base::getModel());
}

}

//------------------------------------------------------------------------------

namespace Control {

Button::StateDetector::StateDetector (Button* owner)
: m_owner (*owner)
, m_bWasDown (false)
{
}

void Button::StateDetector::buttonClicked (juce::Button* button)
{
}

void Button::StateDetector::buttonStateChanged (juce::Button* button)
{
  if( button->isDown() )
  {
    if( !m_bWasDown )
    {
      m_bWasDown = true;
      m_owner.getModel().pressed();
    }
  }
  else if( m_bWasDown )
  {
    m_bWasDown = false;
    m_owner.getModel().released();
  }
}

Button::Button (Facade::Button* facade,
                ReferenceCountedObjectPtr <Model::Button> model)
: Base (this, facade, model.getObject())
, juce::Button (String::empty)
, m_listener (this)
, m_bEnabledUnboundedMouseMovement (false)
{
  setTriggeredOnMouseDown (true);
  addListener (&m_listener);

  getModel().addView (this);
  getModel().addListener (this);

  setEnabled (getModel().isEnabled ());
}

Button::~Button()
{
  getModel().removeListener (this);
  getModel().removeView (this);
}

Model::Button& Button::getModel ()
{
  return dynamic_cast <Model::Button&> (Base::getModel());
}

Facade::Button& Button::getFacade ()
{
  return dynamic_cast <Facade::Button&> (Base::getFacade());
}

bool Button::isButtonDown ()
{
  return isDown ();
}

bool Button::isMouseOverButton ()
{
  return isOver();
}

void Button::paintButton (Graphics& g,
                          bool isMouseOverButton,
                          bool isButtonDown)
{
  getFacade().setEnabled (juce::Button::isEnabled());
  getFacade().setButtonState (isMouseOverButton, isButtonDown);
  
#if 1
  getFacade().paintFacade (g);
#else
  getFacade().paint (g, getLocalBounds());
#endif
}

void Button::paintOverChildren (Graphics& g)
{
#if 1
  getFacade().paintOverChildren (g);
#else
  getFacade().paintOverChildren (g, getLocalBounds());
#endif
}

void Button::mouseDrag (const MouseEvent& e)
{
  if (juce::Button::isEnabled())
  {
    if (getModel().isMomentary() && !m_bEnabledUnboundedMouseMovement)
    {
      e.source.enableUnboundedMouseMovement (true);
      m_bEnabledUnboundedMouseMovement = true;
    }

    juce::Button::mouseDrag (e);
  }
}

void Button::mouseUp (const MouseEvent& e)
{
  if (juce::Button::isEnabled())
  {
    m_bEnabledUnboundedMouseMovement=false;

    juce::Button::mouseUp (e);
  }
}

void Button::clicked (const ModifierKeys& modifiers)
{
  getModel().clicked (modifiers);
}

void Button::enablementChanged ()
{
  if (isEnabled ())
    getFacade().setAlpha (1.f);
  else
    getFacade().setAlpha (.5f);

  updateView ();
}

void Button::updateView ()
{
#if 0
  setEnabled (getModel().isEnabled ());
  bool enabled = juce::Button::isEnabled ();
  if (getFacade().isEnabled () != enabled)
    getFacade().setEnabled (enabled);

  // make the button look pressed when the model changes without user action
  //setState (getModel().isActive() ? buttonDown :
  //  (isMouseOver() ? buttonOver : buttonNormal));

  repaint();
#endif
}

}

}
