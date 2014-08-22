/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2008 INRIA
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
 * Author: Mohamed Amine Ismail <amine.ismail@sophia.inria.fr>
 */
#include "trace-route-helper.h"
#include "ns3/trace-route.h"
#include "ns3/uinteger.h"
#include "ns3/string.h"

namespace ns3 {


TraceRouteHelper::TraceRouteHelper ()
{
}

TraceRouteHelper::TraceRouteHelper (Address address, uint16_t port)
{
  m_factory.SetTypeId (TraceRoute::GetTypeId ());
  SetAttribute ("RemoteAddress", AddressValue (address));
  SetAttribute ("RemotePort", UintegerValue (port));
}

TraceRouteHelper::TraceRouteHelper (Ipv4Address address, uint16_t port)
{
  m_factory.SetTypeId (TraceRoute::GetTypeId ());
  SetAttribute ("RemoteAddress", AddressValue (Address(address)));
  SetAttribute ("RemotePort", UintegerValue (port));
}

TraceRouteHelper::TraceRouteHelper (Ipv6Address address, uint16_t port)
{
  m_factory.SetTypeId (TraceRoute::GetTypeId ());
  SetAttribute ("RemoteAddress", AddressValue (Address(address)));
  SetAttribute ("RemotePort", UintegerValue (port));
}

void
TraceRouteHelper::SetAttribute (std::string name, const AttributeValue &value)
{
  m_factory.Set (name, value);
}

ApplicationContainer
TraceRouteHelper::Install (NodeContainer c)
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      Ptr<Node> node = *i;
      m_client = m_factory.Create<TraceRoute> ();
      node->AddApplication (m_client);
      apps.Add (m_client);
    }
  return apps;
}

Ptr<TraceRoute>
TraceRouteHelper::GetClient (void)
{
  return m_client;
}




} // namespace ns3
