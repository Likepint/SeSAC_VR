#include "Characters/CVRCharacter.h"
#include "Camera/CameraComponent.h"
#include "MotionControllerComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraDataInterfaceArrayFunctionLibrary.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "Components/CapsuleComponent.h"

ACVRCharacter::ACVRCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	VRCamera = CreateDefaultSubobject<UCameraComponent>("VRCamera");
	VRCamera->SetupAttachment(RootComponent);

	// 모션컨트롤러 컴포넌트 추가
	LHand = CreateDefaultSubobject<UMotionControllerComponent>("LHand");
	LHand->SetupAttachment(RootComponent);
	LHand->SetTrackingMotionSource("Left");

	RHand = CreateDefaultSubobject<UMotionControllerComponent>("RHand");
	RHand->SetupAttachment(RootComponent);
	RHand->SetTrackingMotionSource("Right");

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

	TeleportCircle = CreateDefaultSubobject<UNiagaraComponent>("TeleportCircle");
	TeleportCircle->SetupAttachment(RootComponent);

	// TeleportUI
	TeleportUIComponent = CreateDefaultSubobject<UNiagaraComponent>("TeleportUIComponent");
	TeleportUIComponent->SetupAttachment(RootComponent);

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

	// 텔레포트 활성화시 처리
	if (bTeleporting == true)
	{
		// 텔레포트 그리기
		if (bTeleportCurve)
		{
			DrawTeleportCurve();
		}
		else
		{
			DrawTeleportStraight();
		}

		// 나이아가라커브가 보여지면 데이터 세팅하자
		UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVector(TeleportUIComponent, TEXT("User.PointArray"), Lines);
	}
}

// Called to bind functionality to input
void ACVRCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	auto PC = GetWorld()->GetFirstPlayerController();
	if (PC)
	{
		auto LocalPlayer = PC->GetLocalPlayer();
		auto SS = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer);
		if (SS)
			SS->AddMappingContext(IMC_Context, 1);

	}

	auto InputSystem = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (InputSystem)
	{
		InputSystem->BindAction(IA_Movement, ETriggerEvent::Triggered, this, &ACVRCharacter::OnMovement);
		InputSystem->BindAction(IA_Look, ETriggerEvent::Triggered, this, &ACVRCharacter::OnLook);

		// 텔레포트
		InputSystem->BindAction(IA_Teleport, ETriggerEvent::Started, this, &ACVRCharacter::TeleportStart);
		InputSystem->BindAction(IA_Teleport, ETriggerEvent::Completed, this, &ACVRCharacter::TeleportEnd);
	}

}

void ACVRCharacter::OnMovement(const FInputActionValue& Values)
{
	FVector2D Scale = Values.Get<FVector2D>();
	//FVector Direction = VRCamera->GetForwardVector() * Scale.X + VRCamera->GetRightVector() * Scale.Y;
	//FVector Direction = FVector(Scale.X, Scale.Y, 0);
	//VRCamera->GetComponentTransform().TransformVector(Direction);
	//AddMovementInput(Direction);
	AddMovementInput(VRCamera->GetForwardVector(), Scale.X);
	AddMovementInput(VRCamera->GetRightVector(), Scale.Y);
}

void ACVRCharacter::OnLook(const FInputActionValue& Values)
{
	if (bUsingMouse == false)
		return;

	FVector2D Scale = Values.Get<FVector2D>();

	AddControllerPitchInput(Scale.Y);
	AddControllerYawInput(Scale.X);

}

void ACVRCharacter::ActiveDebugDraw()
{
	bIsDebugDraw = !bIsDebugDraw;

}

// 텔레포트를 설정을 초기화
// 현재 텔레포트가 가능한지도 결과로 넘겨주자
bool ACVRCharacter::ResetTeleport()
{
	// 현재 텔레포트 써클이 보여지고 있으면 이동가능
	// 그렇지 않으면 NO
	bool bCanTeleprot = TeleportCircle->GetVisibleFlag();
	// 텔레포트 종료
	bTeleporting = false;
	TeleportCircle->SetVisibility(false);
	TeleportUIComponent->SetVisibility(false);

	return bCanTeleprot;

}

void ACVRCharacter::TeleportStart(const FInputActionValue& Values)
{
	bTeleporting = true;
	TeleportUIComponent->SetVisibility(true);

}

void ACVRCharacter::TeleportEnd(const FInputActionValue& Values)
{
	// 텔레포트가 가능하지 않으면
	if (ResetTeleport() == false)
	{
		// 아무처리하지 않는다.
		return;
	}

	// 이동
	if (IsWarp)
		DoWarp();
	else SetActorLocation(TeleportLocation);
}

void ACVRCharacter::DrawTeleportStraight()
{
	// 1. Line 을 만들기
	FVector StartPoint = RHand->GetComponentLocation();
	FVector EndPoint = StartPoint + RHand->GetForwardVector() * 1000;
	// 2. Line 을 쏘기
	bool bHit = CheckHitTeleport(StartPoint, EndPoint);

	Lines.Empty();
	Lines.Add(StartPoint);
	Lines.Add(EndPoint);

	//// 선그리기
	//DrawDebugLine(GetWorld(), StartPoint, EndPoint, FColor::Red, false, -1, 0, 1);
	//if(bIsDebugDraw)
	//	DrawDebugSphere(GetWorld(), StartPoint,  200, 20, FColor::Yellow);
}

void ACVRCharacter::DrawTeleportCurve()
{
	// 비워줘야한다.
	Lines.Empty();
	// 선이 진행될 힘(방향)
	FVector Velocity = RHand->GetForwardVector() * CurveForce;
	// P0
	FVector Pos = RHand->GetComponentLocation();
	Lines.Add(Pos);
	// 40번 반복하겠다.
	for (int i = 0; i < LineSmooth; i++)
	{
		FVector LastPos = Pos;
		// v = v0 + at
		Velocity += FVector::UpVector * Gravity * SimulateTime;
		// P = P0 + vt
		Pos += Velocity * SimulateTime;

		bool bHit = CheckHitTeleport(LastPos, Pos);
		// 부딪혔을 때 반복 중단
		Lines.Add(Pos);

		if (bHit)
			break;
	}

	//int LineCount = Lines.Num();
	//for (int i = 0; i < LineCount - 1; i++)
	//{
	//	DrawDebugLine(GetWorld(), Lines[i], Lines[i + 1], FColor::Red, false, -1, 0, 1);
	//}
}

bool ACVRCharacter::CheckHitTeleport(FVector LastPos, FVector& CurPos)
{
	FHitResult HitInfo;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	bool bHit = GetWorld()->LineTraceSingleByChannel(HitInfo, LastPos, CurPos, ECC_Visibility, Params);
	// 3. Line 과 부딪혔다면
	// 4. 그리고 부딪힌 녀석의 이름에 Floor 가 있다면

	if (bHit && HitInfo.GetActor()->GetActorNameOrLabel().Contains("GroundFloor") == true)
	{
		// 텔레포트 UI 활성화
		TeleportCircle->SetVisibility(true);
		// -> 부딪힌 지점에 텔레포트 써클 위치시키기
		TeleportCircle->SetWorldLocation(HitInfo.Location);

		// 텔레포트 위치 지정
		TeleportLocation = HitInfo.Location;

		CurPos = TeleportLocation;
	}
	// 4. 안부딪혔으면
	else
	{
		// -> 써클 안그려지게 하기
		TeleportCircle->SetVisibility(false);
	}
	return bHit;

}

void ACVRCharacter::DoWarp()
{
	// 1. 워프 활성화 되어 있을 때만 수행
	if (IsWarp == false)
		return;

	// 2. 일정시간동안 정해진 위치로 이동하고 싶다.
	CurrentTime = 0;
	// 충돌체 꺼주자
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	GetWorldTimerManager().SetTimer(WarpTimer, FTimerDelegate::CreateLambda(
		[this]()
		{
			// 2.1 시간이 흘러야 
			CurrentTime += GetWorld()->DeltaTimeSeconds;
			// 2.2 warp time 까지 이동하고 싶다.
			// target
			FVector TargetPos = TeleportLocation + FVector::UpVector * GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
			// 현재위치
			FVector CurPos = GetActorLocation();
			// 2.3 이동하고 싶다.
			CurPos = FMath::Lerp(CurPos, TargetPos, CurrentTime / WarpTime);
			SetActorLocation(CurPos);

			// 목적지에 도착했다면
			if (CurrentTime - WarpTime >= 0)
			{
				// -> 목적지 위치로 위치 보정
				SetActorLocation(TargetPos);
				// -> 타이머 끄기
				GetWorldTimerManager().ClearTimer(WarpTimer);
				// -> 충돌체 다시 활성화
				GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			}
		}
	), 0.02f, true);

}
