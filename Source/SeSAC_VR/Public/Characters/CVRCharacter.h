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

	// Motion Controller
	UPROPERTY(VisibleAnywhere, Category = "MotionController")
	class UMotionControllerComponent* LHand;

	UPROPERTY(VisibleAnywhere, Category = "MotionController")
	class UMotionControllerComponent* RHand;

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

	UPROPERTY(VisibleAnywhere)
	bool bUsingMouse = true;

private: // Teleport
	UPROPERTY(EditDefaultsOnly, Category = "Enhanced")
	class UInputAction* IA_Teleport;

	UPROPERTY(EditDefaultsOnly)
	class UNiagaraComponent* TeleportCircle;

	//class UStaticMeshComponent* TeleportCircle;

	// 텔레포트 진행여부
	bool bTeleporting = false;

	UFUNCTION(Exec)
	void ActiveDebugDraw();

	bool bIsDebugDraw = false;

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

	bool CheckHitTeleport(FVector InPrevPoint, FVector& InCurrentPoint);

public: // Teleport UI
	UPROPERTY(VisibleAnywhere)
	class UNiagaraComponent* TeleportUIComponent;

private: // Warp
	// Warp 활성화 여부
	UPROPERTY(EditAnywhere, Category = "Warp", meta = (AllowPrivateAccess = true))
	bool IsWarp = true;

	// 1. 등속 -> 정해진 시간에 맞춰 이동하기
	// 경과 시간
	float CurrentTime = 0;

	// 워프 타임
	float WarpTime = 0.2;

	FTimerHandle WarpTimer;

	// 워프를 수행할 함수
	void DoWarp();

};
