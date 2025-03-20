#include "Characters/CVRCharacter.h"
#include "Camera/CameraComponent.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"

ACVRCharacter::ACVRCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	VRCamera = CreateDefaultSubobject<UCameraComponent>("VRCamera");
	VRCamera->SetupAttachment(RootComponent);

	static ConstructorHelpers::FObjectFinder<UInputMappingContext> IMC_CONTEXT(TEXT("/Script/EnhancedInput.InputMappingContext'/Game/Inputs/IMC_Context.IMC_Context'"));
	if (IMC_CONTEXT.Succeeded())
		IMC_Context = IMC_CONTEXT.Object;

	static ConstructorHelpers::FObjectFinder<UInputAction> IA_MOVEMENT(TEXT("/Script/EnhancedInput.InputAction'/Game/Inputs/IA_Movement.IA_Movement'"));
	if (IA_MOVEMENT.Succeeded())
		IA_Movement = IA_MOVEMENT.Object;

	static ConstructorHelpers::FObjectFinder<UInputAction> IA_LOOK(TEXT("/Script/EnhancedInput.InputAction'/Game/Inputs/IA_Look.IA_Look'"));
	if (IA_LOOK.Succeeded())
		IA_Look = IA_LOOK.Object;

	static ConstructorHelpers::FObjectFinder<UInputAction> IA_TELEPORT(TEXT("/Script/EnhancedInput.InputAction'/Game/Inputs/IA_Teleport.IA_Teleport'"));
	if (IA_TELEPORT.Succeeded())
		IA_Teleport = IA_TELEPORT.Object;

	TeleportCircle = CreateDefaultSubobject<UStaticMeshComponent>("TeleportCircle");
	//TeleportCircle->SetupAttachment(RootComponent);

}

void ACVRCharacter::BeginPlay()
{
	Super::BeginPlay();

	// 게임을 시작할 때 텔레포트가 가능하도록 리셋
	ResetTeleport();

}

void ACVRCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 텔레포트 활성화 시 처리
	if (bTeleporting)
	{
		if (bTeleportCurve)
			DrawTeleportCurve();

		// 텔레포트 그리기
		DrawTeleportStraight();
	}
}

void ACVRCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Cast vs. CastChecked
	// Cast -> 부모, 자식과의 관계가 없다면 nullptr 반환
	// CastChecked -> nullptr가 아니라 crash (assert)
	if (auto pc = GetWorld()->GetFirstPlayerController())
	{
		if (auto subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(pc->GetLocalPlayer()))
			subsystem->AddMappingContext(IMC_Context, 1);
	}

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Movement
		EnhancedInputComponent->BindAction(IA_Movement, ETriggerEvent::Triggered, this, &ACVRCharacter::OnMovement);

		// Look
		EnhancedInputComponent->BindAction(IA_Look, ETriggerEvent::Triggered, this, &ACVRCharacter::OnLook);

		// Teleport
		EnhancedInputComponent->BindAction(IA_Teleport, ETriggerEvent::Started, this, &ACVRCharacter::TeleportStart);
		EnhancedInputComponent->BindAction(IA_Teleport, ETriggerEvent::Completed, this, &ACVRCharacter::TeleportEnd);

	}

}

void ACVRCharacter::OnMovement(const FInputActionValue& InVal)
{
	AddMovementInput(VRCamera->GetForwardVector(), InVal.Get<FVector2D>().X);

	AddMovementInput(VRCamera->GetRightVector(), InVal.Get<FVector2D>().Y);

}

void ACVRCharacter::OnLook(const FInputActionValue& InVal)
{
	AddControllerYawInput(InVal.Get<FVector>().X);

	AddControllerPitchInput(InVal.Get<FVector>().Y);

}

// 텔레포트 설정을 초기화
// 현재 텔레포트가 가능한지 리턴
bool ACVRCharacter::ResetTeleport()
{
	// 현재 텔레포트 써클이 보여지고 있다면 True, 아니라면 false 리턴
	bTeleporting = false;

	TeleportCircle->SetVisibility(false);

	return !TeleportCircle->GetVisibleFlag();
}

void ACVRCharacter::TeleportStart(const FInputActionValue& InVal)
{
	bTeleporting = true;

}

void ACVRCharacter::TeleportEnd(const FInputActionValue& InVal)
{
	// 텔레포트가 불가능하면 처리 X
	if (!ResetTeleport()) return;

	// 이동
	if (TeleportLocation == FVector::ZeroVector) return;
	
	SetActorLocation(TeleportLocation);
	TeleportLocation = FVector::ZeroVector;

}

void ACVRCharacter::DrawTeleportStraight()
{
	// Line 생성
	FVector start = VRCamera->GetComponentLocation();
	FVector end = start + VRCamera->GetForwardVector() * 1000;

	// Line 충돌 시 정보를 받을 변수 생성
	FHitResult info;

	// Character 예외 처리
	FCollisionQueryParams params;
	params.AddIgnoredActor(this);

	// Line 쏘기
	bool bHit = GetWorld()->LineTraceSingleByChannel(info, start, end, ECC_Visibility, params);

	// Line과 충돌하지 않았다면 리턴
	if (!bHit) return;

	// Line과 충돌했다면
	// 텔레포트 UI 활성화, 부딪힌 지점에 텔레포트 써클 위치 표시
	if (info.GetActor()->GetActorNameOrLabel().Contains("GroundFloor"))
	{
		TeleportCircle->SetVisibility(true);

		TeleportCircle->SetWorldLocation(info.ImpactPoint);

		// Line 그리기
		DrawDebugLine(GetWorld(), start, end, FColor::Green, false, -1, 0, 1);

		// 텔레포트 위치 지정
		TeleportLocation = info.Location;
	}

}

void ACVRCharacter::DrawTeleportCurve()
{
	// 반복하기 전 배열의 초기화
	Lines.Empty();

	// 선이 진행될 힘(방향)
	FVector vel = VRCamera->GetForwardVector() * CurveForce;
	FVector p0 = VRCamera->GetComponentLocation();

	Lines.Add(p0);

	// LineSmooth의 값만큼 반복
	for (int32 i = 0; i < LineSmooth; ++i)
	{
		FVector end = p0;

		// v = v0 + at
		vel += FVector::UpVector * Gravity * SimulateTime;

		// P = P0 + vt
		p0 += vel * SimulateTime;

		// Lines.Add(vertex)
		Lines.Add(p0);

	}

	for (int32 i = 0; i < LineSmooth - 1; ++i)
		DrawDebugLine(GetWorld(), Lines[i], Lines[i + 1], FColor::Red, false, -1, 0, 1);

}
