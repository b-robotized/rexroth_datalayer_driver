#include "ctrlx_data_layer_hw_interface/ctrlx_data_layer_hw_interface.hpp"

#include <limits>

namespace ctrlx_data_layer_hw_interface
{
hardware_interface::CallbackReturn CtrlxDataLayerHwInterface::on_init(
    const hardware_interface::HardwareInfo &info)
{
    if (
        hardware_interface::SystemInterface::on_init(info) !=
        hardware_interface::CallbackReturn::SUCCESS)
    {
        return hardware_interface::CallbackReturn::ERROR;
    }

    return hardware_interface::CallbackReturn::SUCCESS;
}

hardware_interface::CallbackReturn CtrlxDataLayerHwInterface::on_configure(
    const rclcpp_lifecycle::State & /*previous_state*/)
{
    RCLCPP_INFO(rclcpp::get_logger("CtrlxDataLayerHwInterface"), "Configuring ...");

    // Beginning of code for ctrlX Data Layer
    // TODO(Manuel) Add setting of initial values
    RCLCPP_INFO(
        rclcpp::get_logger("CtrlxDataLayerHwInterface"), "Starting ctrlX Data Layer system (without broker)");
    datalayerSystem_ = new comm::datalayer::DatalayerSystem;
    datalayerSystem_->start(false); 
    RCLCPP_INFO(
        rclcpp::get_logger("CtrlxDataLayerHwInterface"), "DatalayerSystem created. Get connection string.");

    auto connectionString = getConnectionString();
    RCLCPP_INFO(
        rclcpp::get_logger("CtrlxDataLayerHwInterface"), "Connection string is: %s ...", connectionString.c_str());

    datalayerClientSharedPtr_ = datalayerSystem_->factory()->createClient3(connectionString);
    RCLCPP_INFO(
        rclcpp::get_logger("CtrlxDataLayerHwInterface"), "Created client to data layer.");

    RCLCPP_INFO(
        rclcpp::get_logger("CtrlxDataLayerHwInterface"), "Checking if connected ...");

    if(!datalayerClientSharedPtr_->isConnected())
    {
        RCLCPP_ERROR(
            rclcpp::get_logger("CtrlxDataLayerHwInterface"), "Failed to connect to Data Layer");
        return hardware_interface::CallbackReturn::ERROR;
    }
    RCLCPP_INFO(
        rclcpp::get_logger("CtrlxDataLayerHwInterface"), "Connected to datalayer");

    RCLCPP_INFO(
        rclcpp::get_logger("CtrlxDataLayerHwInterface"), "Get initial values from Datalayer for the interfaces.");

    for (auto & [name, dl_type] : state_interface_to_states_dl_)
    {
        auto ret = read_from_datalayer(name, dl_type);
        if(ret != hardware_interface::return_type::OK)
        {
            RCLCPP_INFO(
                rclcpp::get_logger("CtrlxDataLayerHwInterface"), "Failed to get initial value for CommandInteface<%s>.", name.c_str());
            return hardware_interface::CallbackReturn::ERROR;
        }
        RCLCPP_INFO(
            rclcpp::get_logger("CtrlxDataLayerHwInterface"), "Get initial values from StateInteface<%s> which is:{%f}", name.c_str(), dl_type.value);
    }

    for (auto & [name, dl_type] : command_interface_to_commands_dl_)
    {
        auto ret = read_from_datalayer(name, dl_type);
        if(ret != hardware_interface::return_type::OK)
        {
            RCLCPP_INFO(
                rclcpp::get_logger("CtrlxDataLayerHwInterface"), "Failed to get initial value for CommandInteface<%s>.", name.c_str());
            return hardware_interface::CallbackReturn::ERROR;
        }
        RCLCPP_INFO(
            rclcpp::get_logger("CtrlxDataLayerHwInterface"), "Get initial values from CommandInteface<%s> which is:{%f}", name.c_str(), dl_type.value);
    }

    RCLCPP_INFO(
        rclcpp::get_logger("CtrlxDataLayerHwInterface"), "Successfull configure the CtrlxDataLayerHwInterface.");

    return hardware_interface::CallbackReturn::SUCCESS;
}

std::vector<hardware_interface::StateInterface> CtrlxDataLayerHwInterface::export_state_interfaces()
{
    std::vector<hardware_interface::StateInterface> state_interfaces;
    RCLCPP_INFO(rclcpp::get_logger("CtrlxDataLayerHwInterface"), "State interfaces:");

    const std::string datalayer_base_address = info_.hardware_parameters["datalayer_base_address"];
    for (hardware_interface::ComponentInfo gpio : info_.gpios)
    {
        const std::string device_name = gpio.name;
        for (auto & state_if : gpio.state_interfaces)
        {
            std::string address = "";
            // TODO(Anyone) REMOVE THIS DIRTY DIRTY DRTY HACK!!!!!!!
            if(device_name == "EL2008")
            {
                address = datalayer_base_address +  "output/data/" + device_name + state_if.parameters["variable_name"];
            } else
            {
                address = datalayer_base_address +  datalayer_data_input_ + device_name + state_if.parameters["variable_name"];
            }

            const std::string full_qualified_state_if_name = device_name + "/" + state_if.name;
            // how a full address should look like an example
            // address = fieldbuses/ethercat/master/instances/Beckhof_IOs/realtime_data/input/data/EL1859/Channel_1.Input
            // EL1859/read_di, fieldbuses/ethercat/master/instances/Beckhof_IOs/realtime_data/input/data/EL1859/Channel_1.Input
            // where : 
            // datalayer_base_address               = fieldbuses/ethercat/master/instances/Beckhof_IOs/realtime_data/
            // datalayer_data_input_                = input/data/
            // device_name                          = EL1859
            //state_if.parameters["variable_name"]  = /Channel_1.Input
            
            if (state_if.data_type == "bool")
            {
                auto type = comm::datalayer::VariantType::BOOL8;
                state_interface_to_states_dl_.emplace(std::make_pair(full_qualified_state_if_name, CtrlxDatalayerType(std::numeric_limits<double>::quiet_NaN(), type, address)));
                RCLCPP_INFO(rclcpp::get_logger("CtrlxDataLayerHwInterface"), "creating a StateInteface for type <bool> that maps from [%s, %s]", full_qualified_state_if_name.c_str(), state_interface_to_states_dl_.at(full_qualified_state_if_name).address().c_str());
            }
            else if (state_if.data_type == "int")
            {
                auto type = comm::datalayer::VariantType::INT64;
                   state_interface_to_states_dl_.emplace(std::make_pair(full_qualified_state_if_name, CtrlxDatalayerType(std::numeric_limits<double>::quiet_NaN(), type, address)));
                RCLCPP_INFO(rclcpp::get_logger("CtrlxDataLayerHwInterface"), "creating a StateInteface for type <int> that maps from [%s, %s]", full_qualified_state_if_name.c_str(), state_interface_to_states_dl_.at(full_qualified_state_if_name).address().c_str());
            }
            else if (state_if.data_type == "double")
            {
                auto type = comm::datalayer::VariantType::FLOAT64;                    
                state_interface_to_states_dl_.emplace(std::make_pair(full_qualified_state_if_name, CtrlxDatalayerType(std::numeric_limits<double>::quiet_NaN(), type, address)));
                RCLCPP_INFO(rclcpp::get_logger("CtrlxDataLayerHwInterface"), "creating a StateInteface for type <double> that maps from [%s, %s]", full_qualified_state_if_name.c_str(), state_interface_to_states_dl_.at(full_qualified_state_if_name).address().c_str());
            }
            else
            {
                std::string error_msg = std::string("Invalid data type <" + state_if.data_type + "> for StateInteface:" + full_qualified_state_if_name);
                RCLCPP_ERROR(
                    rclcpp::get_logger("CtrlxDataLayerHwInterface"), error_msg.c_str());
                throw std::runtime_error(error_msg);
            }

            state_interfaces.emplace_back(hardware_interface::StateInterface(
                device_name, state_if.name, &state_interface_to_states_dl_.at(full_qualified_state_if_name).value));
        }
    }
    RCLCPP_INFO(
                rclcpp::get_logger("CtrlxDataLayerHwInterface"), "successfully exported state interfaces");

    return state_interfaces;
}

std::vector<hardware_interface::CommandInterface> CtrlxDataLayerHwInterface::export_command_interfaces()
{
    std::vector<hardware_interface::CommandInterface> command_interfaces;

    RCLCPP_INFO(rclcpp::get_logger("CtrlxDataLayerHwInterface"), "Command interfaces:");

    const std::string datalayer_base_address = info_.hardware_parameters["datalayer_base_address"];
    for (hardware_interface::ComponentInfo gpio : info_.gpios)
    {
        // TODO(Manuel) Add setting of initial values

        const std::string device_name = gpio.name;
        for (auto & command_if : gpio.command_interfaces)
        {
            const std::string full_qualified_command_if_name = device_name + "/" + command_if.name;
            // how a full address should look like an example
            // address = fieldbuses/ethercat/master/instances/Beckhof_IOs/realtime_data/input/data/EL1859/Channel_1.Input
            // EL1859/read_di, fieldbuses/ethercat/master/instances/Beckhof_IOs/realtime_data/input/data/EL1859/Channel_1.Input
            // where : 
            // datalayer_base_address               = fieldbuses/ethercat/master/instances/Beckhof_IOs/realtime_data/
            // datalayer_data_input_                = input/data/
            // device_name                          = EL1859
            //state_if.parameters["variable_name"]  = /Channel_1.Input
            const auto address = datalayer_base_address +  datalayer_data_output_ + device_name + command_if.parameters["variable_name"];
            if (command_if.data_type == "bool")
            {
                auto type = comm::datalayer::VariantType::BOOL8;                   
                command_interface_to_commands_dl_.emplace(std::make_pair(full_qualified_command_if_name, CtrlxDatalayerType(std::numeric_limits<double>::quiet_NaN(), type, address)));
                RCLCPP_INFO(rclcpp::get_logger("CtrlxDataLayerHwInterface"), "creating a CommandInteface for type <bool> that maps from [%s, %s]", full_qualified_command_if_name.c_str(), command_interface_to_commands_dl_.at(full_qualified_command_if_name).address().c_str());
            }
            else if (command_if.data_type == "int")
            {
                auto type = comm::datalayer::VariantType::INT64;
                command_interface_to_commands_dl_.emplace(std::make_pair(full_qualified_command_if_name, CtrlxDatalayerType(std::numeric_limits<double>::quiet_NaN(), type, address)));
                RCLCPP_INFO(rclcpp::get_logger("CtrlxDataLayerHwInterface"), "creating a CommandInteface for type <int> that maps from [%s, %s]", full_qualified_command_if_name.c_str(), command_interface_to_commands_dl_.at(full_qualified_command_if_name).address().c_str());
            }
            else if (command_if.data_type == "double")
            {
                auto type = comm::datalayer::VariantType::FLOAT64;                 
                command_interface_to_commands_dl_.emplace(std::make_pair(full_qualified_command_if_name, CtrlxDatalayerType(std::numeric_limits<double>::quiet_NaN(), type, address)));
                RCLCPP_INFO(rclcpp::get_logger("CtrlxDataLayerHwInterface"), "creating a CommandInteface for type <double> that maps from [%s, %s]", full_qualified_command_if_name.c_str(), command_interface_to_commands_dl_.at(full_qualified_command_if_name).address().c_str());
            }
            else
            {
                std::string error_msg = std::string("Invalid data type <" + command_if.data_type + "> for CommandInterface:" + full_qualified_command_if_name);
                RCLCPP_ERROR(
                    rclcpp::get_logger("CtrlxDataLayerHwInterface"), error_msg.c_str());
                throw std::runtime_error(error_msg);
            }
            command_interfaces.emplace_back(hardware_interface::CommandInterface(
                device_name, command_if.name, &command_interface_to_commands_dl_.at(full_qualified_command_if_name).value));
        }
    }

    RCLCPP_INFO(
                rclcpp::get_logger("CtrlxDataLayerHwInterface"), "successfully exported command interfaces");

    return command_interfaces;
}

hardware_interface::CallbackReturn CtrlxDataLayerHwInterface::on_activate(
    const rclcpp_lifecycle::State & /*previous_state*/)
{
    // add code to activate
    RCLCPP_INFO(rclcpp::get_logger("CtrlxDataLayerHwInterface"), "Successfully activated!");
    // datalayerSystem_->start(false); // TODO (Sachin) : Check the functionality of the start method
    return hardware_interface::CallbackReturn::SUCCESS;
}

hardware_interface::CallbackReturn CtrlxDataLayerHwInterface::on_deactivate(
    const rclcpp_lifecycle::State & /*previous_state*/)
{
    // add code to deactivate
    // datalayerSystem_->stop(false); // TODO (Sachin) : Check the functionality of the stop method
    RCLCPP_INFO(rclcpp::get_logger("CtrlxDataLayerHwInterface"), "Successfully deactivated!");
    return hardware_interface::CallbackReturn::SUCCESS;
}

hardware_interface::return_type CtrlxDataLayerHwInterface::read_from_datalayer(const std::string& interface_name, CtrlxDatalayerType& datalayer_wrapper)
{

    // check if connection is still there
    if(!datalayerClientSharedPtr_->isConnected())
    {
        RCLCPP_ERROR(
                rclcpp::get_logger("CtrlxDataLayerHwInterface"), "Connection to datalayer lost!");
        return hardware_interface::return_type::ERROR;

    }

    // read data from the data layer
    // TODO(Anyone) Check what we should do if not update
    auto result = datalayerClientSharedPtr_->readSync(datalayer_wrapper.address(), &datalayerValue_);
    if (result != DL_OK)
    {
        RCLCPP_WARN(
            rclcpp::get_logger("CtrlxDataLayerHwInterface"), "Failed to read from Data Layer on address<%s>. Interface <%s> not updated.", datalayer_wrapper.address().c_str(), interface_name.c_str());
        return hardware_interface::return_type::ERROR;
    }
    else 
    {
        if (datalayerValue_.getType() == datalayer_wrapper.get_type())
        {
            datalayer_wrapper.value  = double(datalayerValue_);
        }
        else
        {
            RCLCPP_WARN(
                rclcpp::get_logger("CtrlxDataLayerHwInterface"), "Value of StateInterface <%s> at address: %s has unexpected type: %s. StateInterface not updated.",
                interface_name.c_str(), datalayer_wrapper.address().c_str(), datalayerValue_.typeAsString().c_str());
            return hardware_interface::return_type::ERROR;
        }
    }
    return hardware_interface::return_type::OK;
}


hardware_interface::return_type CtrlxDataLayerHwInterface::read(
    const rclcpp::Time & /*time*/, const rclcpp::Duration & /*period*/)
{
    
    for (auto & [name, dl_type] : state_interface_to_states_dl_)
    {
        auto ret = read_from_datalayer(name, dl_type);
        if(ret != hardware_interface::return_type::OK)
        {
            return ret;
        }
    }

    for (auto & [name, dl_type] : command_interface_to_commands_dl_)
    {
        auto ret = read_from_datalayer(name, dl_type);
        if(ret != hardware_interface::return_type::OK)
        {
            return ret;
        }
    }

    return hardware_interface::return_type::OK;
}

hardware_interface::return_type CtrlxDataLayerHwInterface::write(
    const rclcpp::Time &/*time*/, const rclcpp::Duration & /*period*/)
{


    for (auto & [name, dl_type] : command_interface_to_commands_dl_)
    {
        comm::datalayer::Variant convertedValue = dl_type.getValueAsDatalayerVariant();
        comm::datalayer::DlResult result;
        result = datalayerClientSharedPtr_->writeSync(dl_type.address(), &convertedValue);
        if (STATUS_FAILED(result))
        {
            RCLCPP_WARN(
                rclcpp::get_logger("CtrlxDataLayerHwInterface"), "Failed to write value {%f} of CommandInterface<%s>", dl_type.value, name.c_str());
        }
    }
    return hardware_interface::return_type::OK;
}


hardware_interface::CallbackReturn CtrlxDataLayerHwInterface::on_cleanup(
    const rclcpp_lifecycle::State & /*previous_state*/)
{
    // add code to cleanup
    return hardware_interface::CallbackReturn::SUCCESS;
}

hardware_interface::CallbackReturn CtrlxDataLayerHwInterface::on_error(
    const rclcpp_lifecycle::State & /*previous_state*/)
{
    // add code to handle error
    return hardware_interface::CallbackReturn::SUCCESS;
}
} // namespace ctrlx_data_layer_hw_interface


#include "pluginlib/class_list_macros.hpp"

PLUGINLIB_EXPORT_CLASS(ctrlx_data_layer_hw_interface::CtrlxDataLayerHwInterface, hardware_interface::SystemInterface)
