#pragma once

#include <map>
#include <fstream>
#include <iostream>
#include <filesystem>
#include "tao/json.hpp"
#include "util/String.h"
#include "platform/Logger.h"
#include "Types.h"
#include "platform/Path.h"

const std::map<std::string, ButtonType> ButtonLookup = {
	{ "Left", ButtonType::GB_KEY_LEFT },
	{ "Up", ButtonType::GB_KEY_UP },
	{ "Right", ButtonType::GB_KEY_RIGHT },
	{ "Down", ButtonType::GB_KEY_DOWN },
	{ "A", ButtonType::GB_KEY_A },
	{ "B", ButtonType::GB_KEY_B },
	{ "Start", ButtonType::GB_KEY_START },
	{ "Select", ButtonType::GB_KEY_SELECT }
};

const std::map<std::string, int> KeyLookup = {
	{ "Backspace", 0x08 },
	{ "Tab", 0x09 },
	{ "Clear", 0x0C },
	{ "Enter", 0x0D },
	{ "Shift", 0x10 },
	{ "Ctrl", 0x11 },
	{ "Alt", 0x12 },
	{ "Pause", 0x13 },
	{ "Caps", 0x14 },
	{ "Esc", 0x1B },
	{ "Space", 0x20 },
	{ "PageUp", 0x21 },
	{ "PageDown", 0x22 },
	{ "End", 0x23 },
	{ "Home", 0x24 },
	{ "LeftArrow", 0x25 },
	{ "UpArrow", 0x26 },
	{ "RightArrow", 0x27 },
	{ "DownArrow", 0x28 },
	{ "Select", 0x29 },
	{ "Print", 0x2A },
	{ "Execute", 0x2B },
	{ "PrintScreen", 0x2C },
	{ "Insert", 0x2D },
	{ "Delete", 0x2E },
	{ "Help", 0x2F },
	{ "0", 0x30 },
	{ "1", 0x31 },
	{ "2", 0x32 },
	{ "3", 0x33 },
	{ "4", 0x34 },
	{ "5", 0x35 },
	{ "6", 0x36 },
	{ "7", 0x37 },
	{ "8", 0x38 },
	{ "9", 0x39 },
	{ "A", 0x41 },
	{ "B", 0x42 },
	{ "C", 0x43 },
	{ "D", 0x44 },
	{ "E", 0x45 },
	{ "F", 0x46 },
	{ "G", 0x47 },
	{ "H", 0x48 },
	{ "I", 0x49 },
	{ "J", 0x4A },
	{ "K", 0x4B },
	{ "L", 0x4C },
	{ "M", 0x4D },
	{ "N", 0x4E },
	{ "O", 0x4F },
	{ "P", 0x50 },
	{ "Q", 0x51 },
	{ "R", 0x52 },
	{ "S", 0x53 },
	{ "T", 0x54 },
	{ "U", 0x55 },
	{ "V", 0x56 },
	{ "W", 0x57 },
	{ "X", 0x58 },
	{ "Y", 0x59 },
	{ "Z", 0x5A },
	{ "LeftWin", 0x5B },
	{ "RightWin", 0x5C },
	{ "Sleep", 0x5F },
	{ "NumPad0", 0x60 },
	{ "NumPad1", 0x61 },
	{ "NumPad2", 0x62 },
	{ "NumPad3", 0x63 },
	{ "NumPad4", 0x64 },
	{ "NumPad5", 0x65 },
	{ "NumPad6", 0x66 },
	{ "NumPad7", 0x67 },
	{ "NumPad8", 0x68 },
	{ "NumPad9", 0x69 },
	{ "Multiply", 0x6A },
	{ "Add", 0x6B },
	{ "Separator", 0x6C },
	{ "Subtract", 0x6D },
	{ "Decimal", 0x6E },
	{ "Divide", 0x6F },
	{ "F1", 0x70 },
	{ "F2", 0x71 },
	{ "F3", 0x72 },
	{ "F4", 0x73 },
	{ "F5", 0x74 },
	{ "F6", 0x75 },
	{ "F7", 0x76 },
	{ "F8", 0x77 },
	{ "F9", 0x78 },
	{ "F10", 0x79 },
	{ "F11", 0x7A },
	{ "F12", 0x7B },
	{ "F13", 0x7C },
	{ "F14", 0x7D },
	{ "F15", 0x7E },
	{ "F16", 0x7F },
	{ "F17", 0x80 },
	{ "F18", 0x81 },
	{ "F19", 0x82 },
	{ "F20", 0x83 },
	{ "F21", 0x84 },
	{ "F22", 0x85 },
	{ "F23", 0x86 },
	{ "F24", 0x87 },
	{ "NumLock", 0x90 },
	{ "Scroll", 0x91 },
	{ "LeftShift", 0xA0 },
	{ "RightShift", 0xA1 },
	{ "LeftCtrl", 0xA2 },
	{ "RightCtrl", 0xA3 },
	{ "LeftMenu", 0xA4 },
	{ "RightMenu", 0xA5 },
};

const std::string DEFAULT_CONFIG = "{\"Up\":\"UpArrow\",\"Down\":\"DownArrow\",\"Left\":\"LeftArrow\",\"Right\":\"RightArrow\",\"A\":\"Z\",\"B\":\"X\",\"Start\":\"Enter\",\"Select\":\"Ctrl\"}";

//#define LOG_KEYBOARD_INPUT

class KeyMap {
private:
	std::map<int, int> _keyMap;

public:
	void load() {
		std::string contentPath = getContentPath();
		if (!std::filesystem::exists(contentPath)) {
			std::filesystem::create_directory(contentPath);
		}

		std::string buttonPath = contentPath + "\\buttons.json";
		if (!std::filesystem::exists(buttonPath)) {
			std::ofstream configOut(buttonPath);
			auto defaultConfig = tao::json::from_string(DEFAULT_CONFIG);
			tao::json::to_stream(configOut, defaultConfig, 2);
		}

		tao::json::value buttonConfig;
		if (std::filesystem::exists(buttonPath)) {
			buttonConfig = tao::json::parse_file(buttonPath);
			if (!buttonConfig.is_object()) {
				buttonConfig = tao::json::from_string(DEFAULT_CONFIG);
			}
		} else {
			buttonConfig = tao::json::from_string(DEFAULT_CONFIG);
		}

		for (const auto& button : buttonConfig.get_object()) {
			auto buttonFound = ButtonLookup.find(button.first);
			if (buttonFound == ButtonLookup.end()) {
				std::cout << "Button type '" << button.first << "' unknown" << std::endl;
			} else {
				auto keyFound = KeyLookup.find(button.second.get_string());
				if (keyFound == KeyLookup.end()) {
					std::cout << "Key type '" << button.second.get_string() << "' unknown" << std::endl;
				} else {
					_keyMap[keyFound->second] = (int)buttonFound->second;
				}
			}
		}
	}

	int getControllerButton(int key) const {
#ifdef LOG_KEYBOARD_INPUT
		const std::string* keyName = getKeyName(key);
		if (keyName) {
			consoleLog("Key pressed: " + *keyName);
		} else {
			consoleLog("Key with index " + std::to_string(key) + " not found");
		}
#endif

		auto found = _keyMap.find(key);
		if (found != _keyMap.end()) {
			return found->second;
		}

		return -1;
	}

	const std::string* getKeyName(int idx) const {
		for (auto& key : KeyLookup) {
			if (key.second == idx) {
				return &key.first;
			}
		}

		return nullptr;
	}
};
