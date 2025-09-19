# `rexroth_datalayer_driver`

### Overview

This package provides a non-real-time (NRT) `ros2_control` hardware interface for the **Bosch Rexroth ctrlX CORE** LINK. It allows ROS2 applications to communicate with the **ctrlX Data Layer** LINK, enabling the reading of states and sending of commands to variables exposed on the controller.

This hardware interface is designed to map `ros2_control` interfaces to Data Layer nodes.

---

### Prerequisites

1.  **Bosch Rexroth ctrlX AUTOMATION SDK**: This hardware interface is built against the public SDK. You must have the SDK installed and sourced in your development environment.
    * **[INSERT LINK TO SDK HERE]**

2.  **Execution Environment**: This driver **must** be run on the ctrlX device itself (e.g., a ctrlX CORE or COREvirtual), so the datalayer is instantiated.

3.  **ROS2 Control Snap**: The current version of this driver relies on Inter-Process Communication (IPC) to connect to the Data Layer. The `datalayer` SDK supports TCP, but due to firewall and security reasons, it is not enabled. Therefore, your `ros2_control` node must be packaged and run as a snap on the ctrlX device to have access to the necessary communication channels.

---

### Installation

1.  Clone this repository into your ROS2 workspace:
    ```bash
    git clone [INSERT REPO URL HERE] src/rexroth_datalayer_driver
    ```

2.  Build the workspace:
    ```bash
    colcon build --packages-select datalayer_hardware_interface_nrt --cmake-args -DBUNDLE_SDK_DEP_RPATH=/my/path/to/lib
    ```

  During build, `BUNDLE_SDK_DEP_RPATH` must be specified. This ensures that the compiled hardware interface `.so` can find the required Data Layer libraries (`libcomm_datalayer.so`) at runtime.

3.  Source the workspace:
    ```bash
    source install/setup.bash
    ```
---

### Configuration and Usage

The hardware interface is configured through the URDF file loaded by the `ros2_control` controller manager. You need to define the Data Layer connection parameters and map the command/state interfaces to specific Data Layer nodes.

#### `ros2_control` Tag Parameters

* `device_ip`: The IP address of the ctrlX CORE. (Currently ignored in favor of IPC).
* `device_user`: Username for the Data Layer connection.
* `device_password`: Password for the Data Layer connection.

#### Interface Parameters

For each `<command_interface>` or `<state_interface>`, you must specify the following parameters:

* `DL_address`: The base address of the node in the Data Layer.
* `DL_variable`: The specific variable name under the base address.
* `DL_variable_type`: The data type of the variable.

#### Supported Data Types

The hardware interface currently supports the following `comm::datalayer::VariantType` values:
* `BOOL8`
* `INT64`
* `UINT64`
* `FLOAT64`

#### Example URDF Configuration:
Example urdf configuration can be found in the accompanying `datalayer_bringup` package LINK.

Note: for now, ***Only `<gpio>` component is supported!***

```xml
<ros2_control name="DataLayerHardwareInterface" type="system">
    <hardware>
        <plugin>datalayer_hardware_interface_nrt/DataLayerHardwareInterface_NRT</plugin>
        <param name="device_ip">192.168.1.1</param>
        <param name="device_user">boschrexroth</param>
        <param name="device_password">boschrexroth</param>
    </hardware>
    <gpio name="datalayer">
        <command_interface name="some_output">
            <param name="DL_address">fieldbuses/master/instances/my_io</param>
            <param name="DL_variable">output/data/my_variable</param>
            <param name="DL_variable_type">BOOL8</param>
        </command_interface>
        <state_interface name="scheduler_counter">
            <param name="DL_address">scheduler/admin/info/common-data-rt</param>
            <param name="DL_variable">counter</param>
            <param name="DL_variable_type">UINT64</param>
        </state_interface>w

    </gpio>
</ros2_control>
```

### Development Roadmap

This hardware interface is under active development. Future improvements include:

* **Dynamic Type Detection**: Instead of requiring `DL_variable_type` in the URDF, the interface will query the metadata of a Data Layer node during the `on_configure` step to determine its type automatically.
* **Expanded Interface Support**: support for `<sensor>`, `<joint>`, and `<gpio>` URDF components.
* **Broader DatalayerType Support**: Add handlers for more Data Layer variant types (e.g., `FLOAT32`, `INT32`, arrays, etc.).
* **Initial Value**: Implement a mechanism to set initial values for command interfaces on startup.
* **Lifecycle Management**: Move the Data Layer connection logic from `on_configure` to `on_activate` and `on_deactivate` to better align with `ros2_control` lifecycle management. This will allow for cleaner activation and deactivation of the hardware.