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

public:
	ACVRCharacter();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private: // Enhanced Input
	UPROPERTY(EditDefaultsOnly, Category = "Enhanced")
	class UInputMappingContext* IMC_Context;

private: // Movement
	UPROPERTY(EditDefaultsOnly, Category = "Enhanced")
	class UInputAction* IA_Movement;

	void OnMovement(const struct FInputActionValue& InVal);

private: // Look
	UPROPERTY(EditDefaultsOnly, Category = "Enhanced")
	class UInputAction* IA_Look;

	void OnLook(const struct FInputActionValue& InVal);

private: // Teleport
	UPROPERTY(EditDefaultsOnly, Category = "Enhanced")
	class UInputAction* IA_Teleport;

	UPROPERTY(EditDefaultsOnly)
	class UStaticMeshComponent* TeleportCircle;

	// 텔레포트 진행여부
	bool bTeleporting = false;

	// 텔레포트 리셋
	bool ResetTeleport();

	void TeleportStart(const struct FInputActionValue& InVal);
	void TeleportEnd(const struct FInputActionValue& InVal);

	void DrawTeleportStraight();

	// 텔레포트 위치
	FVector TeleportLocation;

private: // Curve Teleport
		 // P = P0 + vt
		 // v = v0 + at
		 // F = ma
	UPROPERTY(EditAnywhere) // 곡선을 이루는 점의 개수(곡선의 부드럽기 정도)
	int LineSmooth = 40;

	UPROPERTY(EditAnywhere)
	float CurveForce = 1500;

	UPROPERTY(EditAnywhere) // 중력가속도
	float Gravity = -5000;

	UPROPERTY(EditAnywhere) // DeltaTime
	float SimulateTime = 0.02;

	// 기억할 점 리스트
	TArray<FVector> Lines;

	// 텔레포트 모드 전환 (Curve or not)
	UPROPERTY(EditAnywhere)
	bool bTeleportCurve = true;

	// 곡선 텔레포트 그리기
	void DrawTeleportCurve();

};
