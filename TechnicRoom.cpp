#include "TechnicRoom.h"
#include <chrono>
#include <thread>
#include <wiringPi.h>
#include "IDevice.h"
#include "ElektrolytPump.h"
#include "Inverter.h"
#include "Logger.h"
#include "TimeoutClock.h"
#include "Ventilation.h"

TechnicRoom::TechnicRoom() {
	wiringPiSetup();
	m_pInv1 = std::make_shared<Inverter>("/dev/ttyPORT1");
	m_pInv2 = std::make_shared<Inverter>("/dev/ttyPORT2");
	m_pInv3 = std::make_shared<Inverter>("/dev/ttyPORT3");
	m_pInv4 = std::make_shared<Inverter>("/dev/ttyPORT4Udevadm info starts with the device specified by the devpath and then
walks up the chain of parent devices. It prints for every device
found, all possible attributes in the udev rules key format.
A rule to match, can be composed by the attributes of the device
and the attributes from one single parent device.

  looking at device '/devices/platform/soc/3f980000.usb/usb1/1-1/1-1.4/1-1.4.4/1                                                                                                                                                             -1.4.4:1.0/ttyUSB0/tty/ttyUSB0':
    KERNEL=="ttyUSB0"
    SUBSYSTEM=="tty"
    DRIVER==""

  looking at parent device '/devices/platform/soc/3f980000.usb/usb1/1-1/1-1.4/1-                                                                                                                                                             1.4.4/1-1.4.4:1.0/ttyUSB0':
    KERNELS=="ttyUSB0"
    SUBSYSTEMS=="usb-serial"
    DRIVERS=="pl2303"
    ATTRS{port_number}=="0"

  looking at parent device '/devices/platform/soc/3f980000.usb/usb1/1-1/1-1.4/1-                                                                                                                                                             1.4.4/1-1.4.4:1.0':
    KERNELS=="1-1.4.4:1.0"
    SUBSYSTEMS=="usb"
    DRIVERS=="pl2303"
    ATTRS{bInterfaceProtocol}=="00"
    ATTRS{supports_autosuspend}=="1"
    ATTRS{bInterfaceClass}=="ff"
    ATTRS{bInterfaceNumber}=="00"
    ATTRS{bAlternateSetting}==" 0"
    ATTRS{authorized}=="1"
    ATTRS{bInterfaceSubClass}=="00"
    ATTRS{bNumEndpoints}=="03"

  looking at parent device '/devices/platform/soc/3f980000.usb/usb1/1-1/1-1.4/1-                                                                                                                                                             1.4.4':
    KERNELS=="1-1.4.4"
    SUBSYSTEMS=="usb"
    DRIVERS=="usb"
    ATTRS{bMaxPacketSize0}=="64"
    ATTRS{bDeviceProtocol}=="00"
    ATTRS{bDeviceSubClass}=="00"
    ATTRS{bNumConfigurations}=="1"
    ATTRS{speed}=="12"
    ATTRS{rx_lanes}=="1"
    ATTRS{bDeviceClass}=="00"
    ATTRS{idProduct}=="2303"
    ATTRS{devspec}=="  (null)"
    ATTRS{bMaxPower}=="100mA"
    ATTRS{quirks}=="0x0"
    ATTRS{devpath}=="1.4.4"
    ATTRS{maxchild}=="0"
    ATTRS{idVendor}=="067b"
    ATTRS{bmAttributes}=="a0"
    ATTRS{busnum}=="1"
    ATTRS{devnum}=="12"
    ATTRS{configuration}==""
    ATTRS{removable}=="unknown"
    ATTRS{bConfigurationValue}=="1"
    ATTRS{product}=="USB-Serial Controller"
    ATTRS{authorized}=="1"
    ATTRS{bNumInterfaces}==" 1"
    ATTRS{version}==" 2.00"
    ATTRS{tx_lanes}=="1"
    ATTRS{avoid_reset_quirk}=="0"
    ATTRS{bcdDevice}=="0300"
    ATTRS{ltm_capable}=="no"
    ATTRS{manufacturer}=="Prolific Technology Inc."
    ATTRS{urbnum}=="25"

  looking at parent device '/devices/platform/soc/3f980000.usb/usb1/1-1/1-1.4':
    KERNELS=="1-1.4"
    SUBSYSTEMS=="usb"
    DRIVERS=="usb"
    ATTRS{removable}=="removable"
    ATTRS{devpath}=="1.4"
    ATTRS{bConfigurationValue}=="1"
    ATTRS{devnum}=="5"
    ATTRS{bNumInterfaces}==" 1"
    ATTRS{urbnum}=="149"
    ATTRS{idProduct}=="0610"
    ATTRS{idVendor}=="05e3"
    ATTRS{tx_lanes}=="1"
    ATTRS{version}==" 2.00"
    ATTRS{maxchild}=="4"
    ATTRS{bMaxPower}=="100mA"
    ATTRS{authorized}=="1"
    ATTRS{bDeviceClass}=="09"
    ATTRS{configuration}==""
    ATTRS{bMaxPacketSize0}=="64"
    ATTRS{bmAttributes}=="e0"
    ATTRS{rx_lanes}=="1"
    ATTRS{bDeviceSubClass}=="00"
    ATTRS{quirks}=="0x0"
    ATTRS{bNumConfigurations}=="1"
    ATTRS{speed}=="480"
    ATTRS{ltm_capable}=="no"
    ATTRS{devspec}=="  (null)"
    ATTRS{product}=="USB2.0 Hub"
    ATTRS{busnum}=="1"
    ATTRS{manufacturer}=="GenesysLogic"
    ATTRS{avoid_reset_quirk}=="0"
    ATTRS{bcdDevice}=="9224"
    ATTRS{bDeviceProtocol}=="02"

  looking at parent device '/devices/platform/soc/3f980000.usb/usb1/1-1':
    KERNELS=="1-1"
    SUBSYSTEMS=="usb"
    DRIVERS=="usb"
    ATTRS{busnum}=="1"
    ATTRS{bDeviceClass}=="09"
    ATTRS{urbnum}=="47"
    ATTRS{idVendor}=="0424"
    ATTRS{bNumInterfaces}==" 1"
    ATTRS{devnum}=="2"
    ATTRS{removable}=="unknown"
    ATTRS{configuration}==""
    ATTRS{tx_lanes}=="1"
    ATTRS{speed}=="480"
    ATTRS{bmAttributes}=="e0"
    ATTRS{bDeviceSubClass}=="00"
    ATTRS{bMaxPower}=="2mA"
    ATTRS{devpath}=="1"
    ATTRS{rx_lanes}=="1"
    ATTRS{bDeviceProtocol}=="02"
    ATTRS{ltm_capable}=="no"
    ATTRS{bcdDevice}=="0200"
    ATTRS{bConfigurationValue}=="1"
    ATTRS{idProduct}=="9514"
    ATTRS{quirks}=="0x0"
    ATTRS{maxchild}=="5"
    ATTRS{avoid_reset_quirk}=="0"
    ATTRS{bNumConfigurations}=="1"
    ATTRS{version}==" 2.00"
    ATTRS{authorized}=="1"
    ATTRS{bMaxPacketSize0}=="64"

  looking at parent device '/devices/platform/soc/3f980000.usb/usb1':
    KERNELS=="usb1"
    SUBSYSTEMS=="usb"
    DRIVERS=="usb"
    ATTRS{urbnum}=="25"
    ATTRS{bNumConfigurations}=="1"
    ATTRS{bcdDevice}=="0419"
    ATTRS{bMaxPower}=="0mA"
    ATTRS{interface_authorized_default}=="1"
    ATTRS{ltm_capable}=="no"
    ATTRS{busnum}=="1"
    ATTRS{bDeviceSubClass}=="00"
    ATTRS{quirks}=="0x0"
    ATTRS{bConfigurationValue}=="1"
    ATTRS{bDeviceProtocol}=="01"
    ATTRS{authorized_default}=="1"
    ATTRS{manufacturer}=="Linux 4.19.97-v7+ dwc_otg_hcd"
    ATTRS{speed}=="480"
    ATTRS{serial}=="3f980000.usb"
    ATTRS{bMaxPacketSize0}=="64"
    ATTRS{idVendor}=="1d6b"
    ATTRS{product}=="DWC OTG Controller"
    ATTRS{removable}=="unknown"
    ATTRS{rx_lanes}=="1"
    ATTRS{configuration}==""
    ATTRS{tx_lanes}=="1"
    ATTRS{avoid_reset_quirk}=="0"
    ATTRS{bNumInterfaces}==" 1"
    ATTRS{maxchild}=="1"
    ATTRS{devpath}=="0"
    ATTRS{version}==" 2.00"
    ATTRS{idProduct}=="0002"
    ATTRS{authorized}=="1"
    ATTRS{bmAttributes}=="e0"
    ATTRS{bDeviceClass}=="09"
    ATTRS{devnum}=="1"

  looking at parent device '/devices/platform/soc/3f980000.usb':
    KERNELS=="3f980000.usb"
    SUBSYSTEMS=="platform"
    DRIVERS=="dwc_otg"
    ATTRS{regvalue}=="invalid offset"
    ATTRS{hcddump}=="HCD Dump"
    ATTRS{srpcapable}=="SRPCapable = 0x1"
    ATTRS{wr_reg_test}=="Time to write GNPTXFSIZ reg 10000000 times: 350 msecs (                                                                                                                                                             35 jiffies)"
    ATTRS{rem_wakeup_pwrdn}==""
    ATTRS{enumspeed}=="Device Enumeration Speed = 0x1"
    ATTRS{remote_wakeup}=="Remote Wakeup Sig = 0 Enabled = 0 LPM Remote Wakeup =                                                                                                                                                              0"
    ATTRS{mode}=="Mode = 0x1"
    ATTRS{mode_ch_tim_en}=="Mode Change Ready Timer Enable = 0x0"
    ATTRS{gpvndctl}=="GPVNDCTL = 0x00000000"
    ATTRS{hnp}=="HstNegScs = 0x0"
    ATTRS{fr_interval}=="Frame Interval = 0x1d4b"
    ATTRS{ggpio}=="GGPIO = 0x00000000"
    ATTRS{devspeed}=="Device Speed = 0x0"
    ATTRS{hnpcapable}=="HNPCapable = 0x1"
    ATTRS{rd_reg_test}=="Time to read GNPTXFSIZ reg 10000000 times: 920 msecs (9                                                                                                                                                             2 jiffies)"
    ATTRS{guid}=="GUID = 0x2708a000"
    ATTRS{gusbcfg}=="GUSBCFG = 0x20001700"
    ATTRS{gnptxfsiz}=="GNPTXFSIZ = 0x01000306"
    ATTRS{buspower}=="Bus Power = 0x1"
    ATTRS{regdump}=="Register Dump"
    ATTRS{hcd_frrem}=="HCD Dump Frame Remaining"
    ATTRS{grxfsiz}=="GRXFSIZ = 0x00000306"
    ATTRS{busconnected}=="Bus Connected = 0x1"
    ATTRS{bussuspend}=="Bus Suspend = 0x0"
    ATTRS{hsic_connect}=="HSIC Connect = 0x1"
    ATTRS{hptxfsiz}=="HPTXFSIZ = 0x02000406"
    ATTRS{srp}=="SesReqScs = 0x1"
    ATTRS{inv_sel_hsic}=="Invert Select HSIC = 0x0"
    ATTRS{regoffset}=="0xffffffff"
    ATTRS{spramdump}=="SPRAM Dump"
    ATTRS{driver_override}=="(null)"
    ATTRS{hprt0}=="HPRT0 = 0x00001005"
    ATTRS{gotgctl}=="GOTGCTL = 0x001c0001"
    ATTRS{gsnpsid}=="GSNPSID = 0x4f54280a"

  looking at parent device '/devices/platform/soc':
    KERNELS=="soc"
    SUBSYSTEMS=="platform"
    DRIVERS==""
    ATTRS{driver_override}=="(null)"

  looking at parent device '/devices/platform':
    KERNELS=="platform"
    SUBSYSTEMS==""
    DRIVERS==""
");

	m_pVentilation = std::make_unique<Ventilation>(*this);
	m_pPump = std::make_unique<ElektrolytPump>(*this);
}
TechnicRoom::~TechnicRoom() = default;


bool TechnicRoom::AnyInverterLoadsTheBattery() const {
	return
		m_pInv1->IsLoadingBattery() ||
		m_pInv2->IsLoadingBattery() ||
		m_pInv3->IsLoadingBattery() ||
		m_pInv4->IsLoadingBattery();
}


void TechnicRoom::Run() {
	{
		std::vector<std::thread> testThreads;
		for (auto pDevice : IDevice::s_devices) {
			testThreads.emplace_back(std::thread{ [pDevice] {pDevice->TestFunctionality(); } });
		}
		for (auto& testThread : testThreads) {
			testThread.join();
		}
	}
	while (true) {
		auto now = std::chrono::high_resolution_clock::now();
		for (auto pDevice : IDevice::s_devices) {
			if (std::chrono::duration_cast<std::chrono::seconds>(now - pDevice->m_lastUpdate).count() > pDevice->m_updateSeconds) {
				pDevice->Update();
				pDevice->m_lastUpdate = now;
			}
		}
		utils::TimeOutClockSeconds loggerFlushClock{std::chrono::seconds{30}};
		if (loggerFlushClock.IsExpired()){
			utils::Logger::Instance().Flush();
			loggerFlushClock.Reset();
		}
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
}
