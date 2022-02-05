// Fill out your copyright notice in the Description page of Project Settings.

#include "ShooterGame.h"
#include "Interfaces/Interactable.h"

// Add default functionality here for any IInteractable functions that are not pure virtual.
bool IInteractable::TryToPickUp(AShooterCharacter* PickInstigator)
{
	return true;
}
