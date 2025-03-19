#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CVRCharacter.generated.h"

UCLASS()
class SESAC_VR_API ACVRCharacter : public ACharacter
{
	GENERATED_BODY()

private: // Components
	UPROPERTY(VisibleAnywhere)
	class UCameraComponent* VRCamera;

private: // Enhanced Input
	UPROPERTY(EditDefaultsOnly, Category = "Enhanced")
	class UInputMappingContext* IMC_Context;

	UPROPERTY(EditDefaultsOnly, Category = "Enhanced")
	class UInputAction* IA_Movement;

	UPROPERTY(EditDefaultsOnly, Category = "Enhanced")
	class UInputAction* IA_Look;

public:
	ACVRCharacter();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	void OnMovement(const struct FInputActionValue& InVal);
	void OnLook(const struct FInputActionValue& InVal);

};
