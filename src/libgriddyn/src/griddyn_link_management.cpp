#include "griddyn_interface.hpp"
#include "griddyn/links/adjustableTransformer.h"

griddyn_bool_t griddyn_link_get_status(griddyn_link const* link)
{
  impl::vet_pointer(link);
  auto const* link_ptr = impl::link_cast_const(link);
  return link_ptr->isConnected();
}

void griddyn_link_set_status(griddyn_link* link, griddyn_bool_t status)
{
  impl::vet_pointer(link);
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
  impl::vet_pointer(link);
  auto const* link_ptr = impl::link_cast_const(link);
  return dynamic_cast<griddyn::links::adjustableTransformer const*>(link_ptr) != nullptr;
}

griddyn_value_t griddyn_link_get_current(griddyn_link const* link)
{
  impl::vet_pointer(link);
  auto const* link_ptr = impl::link_cast_const(link);
  return std::max(link_ptr->getCurrent(0), link_ptr->getCurrent(1));
}

griddyn_value_t griddyn_link_get_rating(griddyn_link const* link)
{
  impl::vet_pointer(link);
  auto const* link_ptr = impl::link_cast_const(link);
  return link_ptr->get("erating");
}
