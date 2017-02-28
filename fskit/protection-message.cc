/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014 Lawrence Livermore National Security, LLC
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Brian M. Kelley <kelley35@llnl.gov>
 */

#include "protection-message.h"
#ifdef GRIDDYN_HAVE_FSKIT
ProtectionMessage::ProtectionMessage ()
  : EventMessage (),
    m_messageType (ProtectionMessage::LOCAL_FAULT_EVENT)
{
}

ProtectionMessage::ProtectionMessage (ProtectionMessage::MESSAGE_TYPE t)
  : EventMessage (),
    m_messageType (t)
{
}

#else
ProtectionMessage::ProtectionMessage ()
  : m_messageType (ProtectionMessage::LOCAL_FAULT_EVENT)
{
}

ProtectionMessage::ProtectionMessage (ProtectionMessage::MESSAGE_TYPE t)
  : m_messageType (t)
{
}

#endif

ProtectionMessage::~ProtectionMessage ()
{
}

ProtectionMessage::MESSAGE_TYPE ProtectionMessage::GetMessageType ()
{
  return m_messageType;
}
