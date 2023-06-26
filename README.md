# embsys-group-project
Project "DreamTeam" contains a telemedicinal application for remote reading of a patients current heart rate.

## Project structure

- HR_Monitor
  *(Core application for heartrate sensor communication and processing)*
  - doc
    *(doxygen generated output)*
- Project_DreamTeamhe
  *(Core application for WIFIBLE communication and webinterface)*
  - doc
    *(doxygen generated output)*
- doc
  *(Various documentation files from the project)*

Detailed summaries/descriptions of the projects are further below.

# WIFIBLE Project (Folder: Project_DreamTeam)

The IoT device is designed to handle heart rate data which are received in real-time, processed, and visualized on a webpage. The communication is managed using the ESP8266 Wi-Fi microchip. The webpage displays the heart rate data in a graphical format as a series of bars and the page auto-updates every 2 seconds.

Part of the project also involved coding in JavaScript, CSS, and HTML to create a dynamic webpage that could handle real-time data and respond to user interaction. The webpage was initially too big to transmit in one request for the ESP8266, so it was optimized and reduced to fit the data constraints.

The software is developed with an emphasis on modular design and has a well-defined structure, which makes the system scalable and easy to maintain. Each module is documented thoroughly with comments, making it easier for developers to understand and modify the code in the future.

The different modules are:

1. Console Module: Manages user interactions on the serial console.
2. Parser Module: Parses user inputs and other incoming data.
3. Heartrate Receiver Module: Communicates with partner device, to receive processed heartrate data (once finger is placed on sensor).
4. Wifible Module: Handles both Wi-Fi and Bluetooth functionalities, manages user  input and requests, and serves the webpage to clients.

Overall, the project is a combination of hardware and software functionalities designed to provide real-time health monitoring with a user-friendly interface and robust backend operations.

## Description

The project revolves around a real-time health monitoring IoT system. The device communicates with a partner MCU via I2C to receive heart rate data. This data is processed in real-time before being displayed to the end-user via a web interface.

The software architecture follows a modular design, with each component functioning independently, thus enhancing the system's reliability and maintainability. Each module can be updated or new features added without affecting the overall system functionality.

The IoT device uses the ESP8266 Wi-Fi microchip to manage network communications. It serves a local webpage to clients, providing a graphical visualization of the received heart rate data.

The webpage is dynamic, designed to update the displayed data in real-time. The heart rate data is represented as a series of bars that change based on the incoming data. The bar's height represents the heart rate value, providing a visual representation of the data to the user.

In addition to the visual representation, the system also supports data parsing and handling of user inputs. These functionalities are managed by separate modules, with each handling a specific task like console interface management, input parsing, Wi-Fi and Bluetooth connectivity, and I2C communication with the partner MCU.

Each of these modules is comprehensively documented, providing a clear description of each function's task. This documentation supports the further development and maintenance of the system.

To summarize, the project offers a robust, user-friendly solution for real-time heart rate monitoring. By leveraging IoT capabilities, it offers practical healthcare monitoring, demonstrating a beneficial use of IoT in healthcare.

# HR_Monitor Project (Folder: HR_Monitor)

The partner project to this IoT system involves developing a real-time heart rate monitoring system that leverages the MAX30101 sensor. This subsystem communicates with the MAX30101, a sophisticated, integrated pulse oximetry and heart rate monitor sensor solution, which directly measures the oxygen saturation and pulse rate from the human body.

This MCU system interfaces with the MAX30101 sensor to gather the analog data and converts it into digital format using an Analog to Digital Converter (ADC). The converted data then goes through a real-time processing algorithm that accurately calculates the heart rate. This calculated heart rate data is then transferred to the primary IoT device through an I2C communication interface.

This partner project plays a crucial role in data acquisition and initial processing. The data it provides is critical for the real-time visualization functionality in the IoT device. The system needs to be highly reliable, accurate, and efficient due to the real-time nature of the data and the criticality of heart rate monitoring.

## Description

The partner project is an integral component of a comprehensive, real-time health monitoring solution. It deals with the essential task of accurately measuring heart rate data and providing it to the main IoT device for further processing and visualization.

The system is based around the MAX30101 sensor, a specialized component designed for pulse oximetry and heart rate measurement. The sensor collects raw analog data which the system then converts into a digital format using an ADC.

Once converted, the digital data is processed in real-time using a sophisticated algorithm to accurately calculate the heart rate. This real-time processing ensures the calculated heart rate data is accurate, up-to-date, and ready for immediate use.

The calculated heart rate data is then sent to the main IoT device via I2C communication, allowing the IoT device to visualize the data for the end-user.

This heart rate monitoring subsystem is engineered for high reliability and accuracy. It is designed with a modular approach, making it scalable and easy to maintain. The well-defined structure and thorough documentation make it easier for developers to understand, modify, and enhance the system in the future. In summary, this partner project provides a reliable and accurate heart rate monitoring solution, forming an essential component of the overall IoT-based healthcare monitoring system.
