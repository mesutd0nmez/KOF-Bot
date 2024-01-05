# KOF-Bot

This project is a cheating program developed for the MMORPG game known as Knight Online. While the **Knight Online** community refers to it as **Koxp**, we have named it **KOF**

After gathering the project dependencies under ```PROJECT_DIR\KOF.Dependencies```, you can compile the project using **Visual Studio 2022**

Project needs [KOF-Server](https://github.com/trkyshorty/KOF-Server) to run, see the KOF-Server [documentation](https://github.com/trkyshorty/KOF-Server/blob/dev/README.md#configuration)

## Table of Contents

- [Dependencies](#dependencies)
- [Protection](#protection)
- [Encryption](#encryption)
- [Compression](#compression)
- [Configuration](#protection)
- [Code Information](#code-information)

## Dependencies

- [cryptopp-8.7.0](https://github.com/weidai11/cryptopp/releases/tag/CRYPTOPP_8_7_0)
- [ImGui-1.90](https://github.com/ocornut/imgui/releases/tag/v1.90)
- [nlohmann-3.11.3](https://github.com/nlohmann/json/releases/tag/v3.11.3)
- [snappy-1.1.10](https://github.com/google/snappy/releases/tag/1.1.10)

## Protection

- [VMProtect 3.8.1](https://vmpsoft.com/)

## Encryption

**KOF-Bot** utilizes **AES-256 CFB** encryption, and the keys for this encryption are configured through the ***KOF.Bot\Service.cpp*** and ***KOF.Bot\Cryption.h*** files

These keys must be **64** characters in length

## Compression

As the default compression, **lzf** is used; however, **snappy** has been added as an option. Snappy has not been used for some time due to issues with **KOF-Bot** on certain older hardware

You can modify compression settings via the ***KOF.Bot\Compression.h*** file

## Configuration

To make changes in the KOF.Bot dynamics and reliability, examine the ***KOF.Bot\Define.h*** file. The definitions here can lead to alterations in the runtime behavior and reliability of the application, which may be necessary during the development process

## Code Information

The `Bot` class in the project serves as the main class, overseeing all aspects of bot management. The `Bot` establishes a connection to the [KOF-Server](https://github.com/trkyshorty/KOF-Server) through the `Service` class internally

After successfully connecting to the game, the `ClientHandler` class takes charge of managing all **Knight Online** processes. Through the `Client` class, `ClientHandler` can read or modify **Knight Online** data

The bot's user interface is created through the ***Drawing*** class, and for interface modifications, refer to the `Drawing` and `UI` classes

This is the structure of the project, and we strive to keep the code organized
