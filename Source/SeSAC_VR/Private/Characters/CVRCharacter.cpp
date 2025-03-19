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

}

void ACVRCharacter::BeginPlay()
{
	Super::BeginPlay();

}

void ACVRCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

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
		EnhancedInputComponent->BindAction(IA_Movement, ETriggerEvent::Triggered, this, &ACVRCharacter::OnMovement);

		EnhancedInputComponent->BindAction(IA_Look, ETriggerEvent::Triggered, this, &ACVRCharacter::OnLook);

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
