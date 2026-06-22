// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:    /room/rathaus/obj/gummigott.c
// Description: nur zur Verwendung durch /i/player/fehler.c und leo.c

inherit "/i/object/nahrung";

void set_vamp_nahrung(int v)
{
    string geschmack;
#ifdef UNItopia
    ::set_vamp_nahrung(v);
#endif
    switch(random(18))
        {
            case  0: geschmack = "Sternenstaub"; break;
            case  1: geschmack = "Erdbeer"; break;
            case  2: geschmack = "Himbeer"; break;
            case  3: geschmack = "Kaffee"; break;
            case  4: geschmack = "Caramel"; break;
            case  5: geschmack = "Vanille"; break;
            case  6: geschmack = "Schokoladen"; break;
            case  7: geschmack = "Petersilien"; break;
            case  8: geschmack = "Schwefel"; break;
            case  9: geschmack = "Alpenkräuter"; break;
            case 10: geschmack = "Zwergenbier"; break;
            case 11: geschmack = "Popcorn"; break;
            case 12: geschmack = "Feendrachenbraten"; break;
            case 13: geschmack = "Göttertränen"; break;
            case 14: geschmack = "Hackfleischbällchen"; break;
            case 15: geschmack = "Lauch"; break;
            case 16: geschmack = "Banananananananen"; break;
            case 17: geschmack = "Ananananananananas"; break;
        }     
    set_long("Ein extra leckeres Gummigöttchen mit "+geschmack+
            "geschmack aus original Pantheon-Produktion. Ein kleiner Lohn "
            "für Deine Mühen.");
    if (v) 
    {
            geschmack = "Blut";
            set_long("Ein, jedenfalls für einen Vampyr, "
                "extra leckeres Gummigöttchen mit "+geschmack+
                "geschmack aus original Pantheon-Produktion. Ein kleiner "
                "Lohn für Deine Mühen.");
    }
    set_smell("Das Gummigöttchen verströmt einen leichten "+
                          geschmack+"geruch.");
    set_success_message("Mit einer wahren "+geschmack+
            "geschmacksexplosion zergeht das Gummigöttchen auf Deiner "
            "Zunge.");
    set_other_success_message("$Der(OBJ_TP) genießt die Früchte "
            "$seines(([name:gummigöttchen,gender:saechlich]),0,OBJ_TP) und "
            "lässt es auf der Zunge zergehen.");
}

void create() {
    ::create();
    set_name("gummigöttchen");
    set_gender("saechlich");
    set_id(({"bonbon", "gummibärchen", "gummigöttchen", 
            "gummigott","fruchtgummi","gummi", "göttchen"}));
    set_material(({"nahrung","gummi"}));
    set_vamp_nahrung(0);
}
