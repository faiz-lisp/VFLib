// Copyright (C) 2008 by Vinnie Falco, this file is part of VFLib.
// See the file README.md for licensing information.

namespace Ui {

namespace Model {

/*
void Listener::onModelChanged (Base* model)
{
}

void Listener::onModelEnablement (Base* model, bool enabled)
{
}
*/

//------------------------------------------------------------------------------

Base::Base ()
  : m_enabled (true)
{
}

Base::~Base ()
{
  jassert (m_views.size()==0); // someone forgot to remove themselves?
}

void Model::Base::destroyConcurrentObject ()
{
  delete this;
}

void Base::addListener (ListenerBase* const listener)
{
  m_listeners.add (listener);
}

void Base::removeListener (ListenerBase* const listener)
{
  m_listeners.remove (listener);
}

bool Base::isEnabled ()
{
  return m_enabled;
}

const String Base::getName ()
{
  return String::empty;
}

void Base::addView (Control::View* view)
{
  if (!m_views.contains (view))
  {
    m_views.add (view);
    view->updateView();
  }
}

void Model::Base::removeView (Control::View* view)
{
  const int index = m_views.indexOf (view);
  if (index >= 0)
    m_views.remove (index);
}

void Model::Base::updateAllViews ()
{
  for (int i = m_views.size(); --i >= 0;)
  {
    m_views.getUnchecked (i)->updateView();
    i = jmin (i, m_views.size());
  }
}

void Model::Base::modelChanged ()
{
  m_listeners.call (&Listener::onModelChanged, (Model::Base*)this);
}

void Model::Base::setEnabled (bool enabled)
{
  if (m_enabled != enabled)
  {
    m_enabled = enabled;

    m_listeners.call (&Listener::onModelEnablement, this);
  }
}

}

}
