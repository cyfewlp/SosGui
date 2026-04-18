ScriptName SosGuiShowEffectScript extends ActiveMagicEffect

Event OnEffectStart(Actor akTarget, Actor akCaster)
    SosGuiNative.ShowSosGuiMenu()
EndEvent

Event OnEffectFinish(Actor akTarget, Actor akCaster)
EndEvent
