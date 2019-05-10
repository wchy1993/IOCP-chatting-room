#pragma once

#include "user.h"

UINT AcceptSoundFunc(LPVOID pParam);
UINT ConnectSoundFunc(LPVOID pParam);
UINT ReceiveSoundFunc(LPVOID pParam);

void StopSound();