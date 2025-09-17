#ifndef CTRLX_DATA_LAYER_HW_INTERFACE__CTRLX_DATA_LAYER_HW_INTERFACE_HPP_
#define CTRLX_DATA_LAYER_HW_INTERFACE__CTRLX_DATA_LAYER_HW_INTERFACE_HPP_

#include <memory>
#include <string>
#include <vector>


#include "hardware_interface/handle.hpp"
#include "hardware_interface/hardware_info.hpp"
#include "hardware_interface/system_interface.hpp"
#include "hardware_interface/types/hardware_interface_return_values.hpp"
#include "rclcpp/macros.hpp"
#include "rclcpp/rclcpp.hpp"
#include "rclcpp_lifecycle/state.hpp"

#include "comm/datalayer/datalayer.h"
#include "comm/datalayer/datalayer_system.h"

namespace datalayer_hardware_interface 
{
//! Retrieve environment variable SNAP
//! @result The content of SNAP ales nullptr if not available
static const char* snapPath()
{
  return std::getenv("SNAP");
}

//! Test if code is runnning in snap environment
//! @result True if running snap environment
static bool isSnap()
{
  return snapPath() != nullptr;
}

//! Get Datalayer connection string
//! @param[in] ip       IP address of the ctrlX CORE: 10.0.2.2 is ctrlX COREvirtual with port forwarding
//! @param[in] user     User name
//! @param[in] password The password
//! @param[in] sslPort  The port number for SSL: 8443 if ctrlX COREvirtual with port forwarding 8443:443
//! @result Connection string
static std::string getConnectionString(
  const std::string& ip = "192.168.1.1",
  const std::string& user = "boschrexroth",
  const std::string& password = "boschrexroth",
  int sslPort = 8443)
{
  if (isSnap())
  {
    return DL_IPC;
  }

  std::string connectionString = DL_TCP + user + std::string(":") + password + std::string("@") + ip;

  if (443 == sslPort)
  {
    return connectionString;
  }

  return connectionString + std::string("?sslport=") + std::to_string(sslPort);
}

class DatalayerType
{
  public:
    explicit DatalayerType(const double& value, const comm::datalayer::VariantType& type, const std::string& address)
      : value(value), type_(type), address_(address)
    {
      if(!is_supported_type(type_))
      {
        throw std::invalid_argument("Failed to initialize the DatalayerType because the type you passed it not supported.");
      }
      if(address_.empty())
      {
        throw std::invalid_argument("Failed to initialize the DatalayerType because of an empty address.");
      }
    }

    bool set_type(const comm::datalayer::VariantType& variant_type)
    {
        if(is_supported_type(variant_type))
        {
            type_ = variant_type;
            return true;
        }
        return false;
    }

    comm::datalayer::VariantType get_type() const
    {
      return type_;
    }

    const std::string& address() const { return address_; }
    
    comm::datalayer::Variant getValueAsDatalayerVariant () const
    {
      comm::datalayer::Variant variant;

      switch (type_)
      {
          case comm::datalayer::VariantType::BOOL8:
          {
              variant.setValue(static_cast<bool>(value));
              break;
          }
          case comm::datalayer::VariantType::INT64:
          {
              // TODO(Manuel) avoid overflow
              variant.setValue(static_cast<int64_t>(round(value)));
              break;
          }
          case comm::datalayer::VariantType::FLOAT64:
          {
              variant.setValue(value);
              break;
          }
          default:
          {
              throw std::runtime_error("Failed to convert to datalayer VariantType because of internal error. The set typ is not supported. Should never happen.");
              break;
          }
      }
      return variant;
    }
    
    double value;
  protected:

    bool is_supported_type(const comm::datalayer::VariantType& variant_type)
    {   
      if(variant_type == comm::datalayer::VariantType::BOOL8 ||
        variant_type == comm::datalayer::VariantType::INT64 ||
        variant_type == comm::datalayer::VariantType::FLOAT64)
        {
          return true;
        }
        return false;
    }

    private:
      comm::datalayer::VariantType type_;
      const std::string address_;
};

class DataLayerHardwareInterface_NRT : public hardware_interface::SystemInterface
{
public:
    hardware_interface::CallbackReturn on_init(
        const hardware_interface::HardwareInfo &info) override;

    hardware_interface::CallbackReturn on_configure(
        const rclcpp_lifecycle::State &previous_state) override;
    
    std::vector<hardware_interface::StateInterface> export_state_interfaces() override;

    std::vector<hardware_interface::CommandInterface> export_command_interfaces() override;

    hardware_interface::CallbackReturn on_activate(
        const rclcpp_lifecycle::State &previous_state) override;
    
    hardware_interface::CallbackReturn on_deactivate(
        const rclcpp_lifecycle::State &previous_state) override;

    hardware_interface::return_type read(
      const rclcpp::Time & time, const rclcpp::Duration & period) override;

    hardware_interface::return_type write(
      const rclcpp::Time & time, const rclcpp::Duration & period) override;

    hardware_interface::CallbackReturn on_cleanup(
      const rclcpp_lifecycle::State & previous_state) override;

    hardware_interface::CallbackReturn on_error(
      const rclcpp_lifecycle::State & previous_state) override;

protected:
  hardware_interface::return_type read_from_datalayer(const std::string& interface_name, DatalayerType& datalayer_wrapper);

private:
  std::unordered_map<std::string, DatalayerType> state_interface_to_states_dl_;
  std::unordered_map<std::string, DatalayerType> command_interface_to_commands_dl_;

  // Shared Ptr for datalayer
  comm::datalayer::IClient3 *datalayerClientSharedPtr_;

  comm::datalayer::DatalayerSystem *datalayerSystem_ = nullptr;
  
  comm::datalayer::Variant datalayerValue_; // TODO (Sachin): change name later

  comm::datalayer::Variant datalayerValueWrite_; 
};
} // namespace datalayer_hardware_interface

#endif  // CTRLX_DATA_LAYER_HW_INTERFACE__CTRLX_DATA_LAYER_HW_INTERFACE_HPP_
