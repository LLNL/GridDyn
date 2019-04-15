#include "griddyn_interface.hpp"
#include "griddyn/links/adjustableTransformer.h"

griddyn_bool_t griddyn_link_get_status(griddyn_link const* link)
{
  auto const* link_ptr = impl::link_cast_const(link);
  return link_ptr->isConnected();
}

void griddyn_link_set_status(griddyn_link* link, griddyn_bool_t status)
{
  auto* link_ptr = impl::link_cast(link);
  if (status)
  {
    link_ptr->reconnect();
  }
  else
  {
    link_ptr->disconnect();
  }
}

griddyn_bool_t griddyn_link_is_transformer(griddyn_link const* link)
{
  auto const* link_ptr = impl::link_cast_const(link);

  bool tap_ne_1 = link_ptr->get("tap") != 1;
  bool is_adjustable_transformer = dynamic_cast<griddyn::links::adjustableTransformer const*>(link_ptr) != nullptr;

  return tap_ne_1 or is_adjustable_transformer;
}

griddyn_value_t griddyn_link_get_current(griddyn_link const* link)
{
  auto const* link_ptr = impl::link_cast_const(link);
  return std::max(link_ptr->getCurrent(0), link_ptr->getCurrent(1));
}

griddyn_value_t griddyn_link_get_rating(griddyn_link const* link)
{
  auto const* link_ptr = impl::link_cast_const(link);
  return link_ptr->get("erating");
}

void griddyn_link_get_buses(griddyn_link const* link, griddyn_bus** bus_1, griddyn_bus** bus_2)
{
  auto const* link_ptr = impl::link_cast_const(link);
  *bus_1 = link_ptr->getBus(1);
  *bus_2 = link_ptr->getBus(2);
}
