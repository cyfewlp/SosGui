Scriptname SosGui extends Quest  

Spell Property ShowSosGui Auto

Event OnInit()
    Actor player = Game.GetPlayer()
    if !player.HasSpell(ShowSosGui)
        player.AddSpell(ShowSosGui)
    endif
EndEvent