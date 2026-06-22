// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:        /apps/description.c
// Description: Aussehen eines Spielers
// Author:      Sissi (20.06.96)
//              Wenn Ihr bei den einzelnen Auspraegungen der Eigenschaften 
//              was hinzufuegt, dann bitte nur hinten dran, niemals
//              "mittenrein".

#include <description.h>

// TODO: Merkmaldefines sollten der Index sein und nicht Index+1
#define INDEX(d)     (d-1)
#define MERKMALE(d)  (merkmale[INDEX(d)])
#define SOURCE(d)    (source[INDEX(d)])
#define MERKMAL(d)   MERKMALE(d)[SOURCE(d)]

mixed *merkmale;
mapping X;


string query_one_description (int mn, int x)
{
    if (x && (mn <= sizeof(merkmale)) && (x < sizeof(MERKMALE(mn))))
        return MERKMALE(mn)[x];
    return 0;
}


void create ()
{
   merkmale = ({

      ({"Augenfarbe","hellblau","blau","dunkelblau","türkis",
                     "hellgrün","dunkelgrün","grün","grünbraun",
                     "hellbraun","dunkelbraun","braun","violett",
                     "giftgrün","dunkelrot","schwarz",
                     "hellgrau","grau","dunkelgrau",
                     "hellrot","rot","bordeauxrot",
                     "weiß","golden","goldfarben","goldig",
                     "bernsteinfarben","graublau"}),
      ({"Augentyp","schmal","rund","groß","klein","mandelförmig",
                   "schlitzäugig","viereckig"}),
      ({"Haarfarbe","hellblond","blond","dunkelblond",
                    "hellbraun","braun","dunkelbraun",
                    "hellrot","rot","dunkelrot",
                    "schwarz","grau","weiß","saphirblau","silbergrau",
                    "hellgrün","grün","dunkelgrün",
                    "hellblau","blau","dunkelblau",
                    "hellorange","orange","dunkelorange",
                    "rotblond"}),
      ({"Haartyp","sind leicht gelockt","sind gelockt","sind stark gelockt",
                  "sind kraus","sind wirr","sind glatt","sind gewellt",
                  "sind stoppelig",
                  "sind zum Zopf geflochten","sind zu Zöpfen geflochten",
                  "sind zu einem Pferdeschwanz gebunden",
                  "sind zu einem Haarkranz geflochten",
                  "sind zu Rastalocken geflochten",
                  "stehen wild in alle Richtungen",
                  "stellen sich nach allen Seiten",
                  "wallen umher",
                  "haben einen Irokesenschnitt"}),
      ({"Haarlänge","sind alle ausgefallen","sind ganz kurze Stoppeln",
                     "sind alle tierisch kurz geschnitten",
                     "reichen bis zu den Ohren",
                     "reichen gerade über die Ohren",
                     "bedecken den Nacken",
                     "reichen bis zu den Schultern",
                     "sind schulterlang",
                     "fallen den Rücken hinunter",
                     "reichen bis auf den Boden hinunter",
                     "fehlen","sind nicht mehr vorhanden"}),
      ({"Nasentyp","kleine Stupsnase","dicke Knollennase","Hakennase",
                   "nasse, kalte Schnauze","ganz normale Nase",
                   "Rübennase","rote Rübennase","messerscharfe Nase",
                   "breite Nase","Plattnase","Adlernase","Triefnase",
                   "runde Kugelnase"}),
      ({"Mundtyp","einen breiten Mund","einen schmalen Mund",
                  "einen kleinen Mund","einen großen Mund",
        	  "einen Schmollmund","einen Mund mit dicken Lippen",
                  "einen ganz normalen Mund","einen Schnabel",
                  "einen großen Schnabel","einen gelben Schnabel",
                  "einen roten Schnabel","gekringelte, schmale Lippen",
                  "einen verkniffenen Mund","ein Fischmaul",
                  "eine süße Schnute","blutrote Lippen",
                  "dunkelrote Lippen"}),
      ({"Ohrentyp","Eselsohren","Schlappohren","spitze Ohren",
                   "kleine Oehrchen","hübsche, geringelte Ohren",
                   "verschnörkelte Ohren","lange Ohren",
                   "schiefe Ohren","Spitzohren","Schlitzohren",
                   "langweilige, normale Ohren","große Ohren",
                   "zerfetzte Ohren","runde Knubbelohren",
                   "leicht abstehende Ohren", "abstehende Ohren",
                   "Segelohren"}),
      ({"Körperbau","ist untersetzt","hat einen Bierbauch",
                     "ist hochgewachsen","ist hünenhaft","ist klein",
                     "ist vom Typ nordischer Kleiderschrank",
                     "hat einen dicken Bauch","ist mollig","ist füllig",
                     "ist mollig rund","ist vollschlank","ist zwergenhaft",
                     "ist schlank","ist dürr","ist schmächtig",
                     "ist pyknisch","ist ein Hänfling","wirkt zerbrechlich",
                     "wirkt sehr zerbrechlich",
                     "sieht sportlich aus","sieht sehr sportlich aus",
                     "ist ein halbes Hemd","ist verkrümmt",
                     "ist bucklig","erinnert an einen Gnom",
                     "sieht aus wie ein Pygmae","sieht aus wie eine Pygmaein",
                     "ist klein und unauffällig",
                     "hat einen Kugelbauch","hat einen Schmerbauch",
                     "erinnert an ein Waschbrett",
                     "ist quadratisch, praktisch, gut"}),
      ({"Haut","hat die Farbe Rot","hat die Farbe Dunkelrot","ist gelb",
               "ist hellgelb","ist von bronzener Farbe","ist schwarz",
               "ist rabenschwarz","ist schwarz wie die Nacht",
               "ist braun","ist dunkelbraun","ist hellbraun","ist gebräunt",
               "ist kreidebleich","ist kreideweiß","sieht kränklich aus",
               "ist blass","ist sehr blass","ist kränklich blass",
               "ist grau","ist hellgrau","ist voller Falten",
               "ist voller Sommersprossen","ist voller kleiner Sommersprossen",
               "ist voller großer Sommersprossen","ist türkis",
               "ist schwarz - weiß geschminkt","ist nachtschwarz","ist grün",
               "ist blau","ist wettergegerbt","ist vom Wetter gegerbt",
               "ist lederartig","ist geschuppt","ist total blau"}),
      ({"Fell","ist rot","ist dunkelrot","ist gelb",
               "ist hellgelb","ist schwarz",
               "ist rabenschwarz","ist schwarz wie die Nacht",
               "ist braun","ist dunkelbraun","ist hellbraun",
               "ist kreideweiß","ist grau","ist hellgrau","ist türkis",
               "ist schwarz mit weißen Flecken",
               "ist weiß mit schwarzen Flecken",
               "ist nachtschwarz","ist grün","ist blau",
               "ist grün mit lila Punkten"}),
   });
   X = ([]);
}


nomask varargs string query_merkmal_string(object player)
{
    string tmp;
    int *source;

    player ||= this_player();

    source = player->query_description_source ();
    if (!source || !sizeof(source)) return 0;
    while (sizeof(source) < sizeof(merkmale)) source += ({0});
    tmp = "";

    // Koerperbau, Hautfarbe
    if (SOURCE(D_KOERPERBAU) && SOURCE(D_HAUT))
        tmp += Er(player)+" "+MERKMAL(D_KOERPERBAU)+", "
              +sein((["name":"haut","gender":"weiblich"]),0,
               player)+" "+        
               MERKMAL(D_HAUT)+". ";
    else if (SOURCE(D_KOERPERBAU) && SOURCE(D_FELL))
        tmp += Er(player)+" "+MERKMAL(D_KOERPERBAU)+", "
              +sein((["name":"fell","gender":"saechlich"]),0,
               player)+" "+        
               MERKMAL(D_FELL)+". ";
    else if (SOURCE(D_KOERPERBAU))
        tmp += Er(player)+" "+MERKMAL(D_KOERPERBAU)+". ";
    else if (SOURCE(D_HAUT))
        tmp += Sein((["name":"haut","gender":"weiblich"]),0,
               player)+" "
               +MERKMAL(D_HAUT)+". ";
    else if (SOURCE(D_FELL))
        tmp += Sein((["name":"fell","gender":"saechlich"]),0,
               player)+" "
               +MERKMAL(D_FELL)+". ";

    // Augen
    if (SOURCE(D_AUGENFARBE) && SOURCE(D_AUGENTYP))
        tmp += Er(player)+" schaut Dich mit "+
               MERKMAL(D_AUGENTYP)+"en, "+
               MERKMAL(D_AUGENFARBE)+"en Augen an. ";
    else if (SOURCE(D_AUGENFARBE))
        tmp += Er(player)+" schaut Dich mit "+
               MERKMAL(D_AUGENFARBE)+"en Augen an. ";
    else if (SOURCE(D_AUGENTYP))
        tmp += Er(player)+" schaut Dich mit "+
               MERKMAL(D_AUGENTYP)+"en Augen an. ";

    // Haare
    if (SOURCE(D_HAARFARBE) && SOURCE(D_HAARTYP) && SOURCE(D_HAARLAENGE))
        tmp += Sein((["name":"haare","gender":"saechlich","plural":1]),0,
               player)+" "+
               MERKMAL(D_HAARLAENGE)+", haben die Farbe "+
               capitalize(MERKMAL(D_HAARFARBE))+" und "+
               MERKMAL(D_HAARTYP)+". ";
    else if (SOURCE(D_HAARFARBE) && SOURCE(D_HAARLAENGE))
        tmp += Sein((["name":"haare","gender":"saechlich","plural":1]),0,
               player)+" "+
               MERKMAL(D_HAARLAENGE)+" und sind von der Farbe "+
               MERKMAL(D_HAARFARBE)+". ";
    else if (SOURCE(D_HAARTYP) && SOURCE(D_HAARLAENGE))
        tmp += Sein((["name":"haare","gender":"saechlich","plural":1]),0,
               player)+" "+
               MERKMAL(D_HAARLAENGE)+" und "+
               MERKMAL(D_HAARTYP)+". ";
    else if (SOURCE(D_HAARFARBE) && SOURCE(D_HAARTYP))
        tmp += Sein((["name":"haare","gender":"saechlich","plural":1]),0,
               player)+" haben die Farbe "+
               capitalize(MERKMAL(D_HAARFARBE))+" und "+
               MERKMAL(D_HAARTYP)+". ";
    else if (SOURCE(D_HAARFARBE))
        tmp += Sein((["name":"haare","gender":"saechlich","plural":1]),0,
               player)+" haben die Farbe "+
               capitalize(MERKMAL(D_HAARFARBE))+". ";
    else if (SOURCE(D_HAARTYP))
        tmp += Sein((["name":"haare","gender":"saechlich","plural":1]),0,
               player)+" "+
               MERKMAL(D_HAARTYP)+". ";
    else if (SOURCE(D_HAARLAENGE))
        tmp += Sein((["name":"haare","gender":"saechlich","plural":1]),0,
               player)+" "+
               MERKMAL(D_HAARLAENGE)+". ";

    // Mund und Nase
    if (SOURCE(D_NASENTYP) && SOURCE(D_MUNDTYP))
        tmp += Er(player)+" hat eine "+
               MERKMAL(D_NASENTYP)+" und "+
               MERKMAL(D_MUNDTYP)+". ";
    else if (SOURCE(D_MUNDTYP))
        tmp += Er(player)+" hat "+
               MERKMAL(D_MUNDTYP)+". ";
    else if (SOURCE(D_NASENTYP))
        tmp += Er(player)+" hat eine "+
               MERKMAL(D_NASENTYP)+". ";

    // Ohren
    if (SOURCE(D_OHRENTYP))
        if (strlen (tmp))
            tmp += "Zwei "+MERKMAL(D_OHRENTYP)+" runden das Bild ab. ";
        else tmp = Er(player)+" hat zwei "+MERKMAL(D_OHRENTYP)+". ";

    if (strlen (tmp))
        return wrap (tmp);
    return 0;
}

static void do_menu ();

void describe_me (string s)
{
    int max;
    if ((this_player() != this_interactive()) ||
        (this_interactive() != previous_object())) return;
    max = this_player()->query_quest_count ();
    if (!max) {
        write ("Um Dich selbst beschreiben zu können, musst Du mindestens "
               "ein Rätsel\ngelöst haben.\n");
        return;
    }
    do_menu ();
}

static void do_menu ()
{
    int i, max, count, merkanz;
    int *source;
    string tmp;
    max = this_player()->query_quest_count();
    source = this_player()->query_description_source ();
    if (source)
        for (i=0; i < sizeof(source); i++)
            if (source[i]) count++;
    if (count >= max) {
        write ("Um ein weiteres Merkmal von Dir beschreiben zu können, "
               "musst Du noch ein\nweiteres Rätsel lösen.\n");
        return;
    }
    merkanz = sizeof(merkmale)-1; // Fell und Haut schliessen sich aus.
    
    if (count >= merkanz) {
        write ("Du hast alle existierenden Merkmale von Dir beschrieben.\n");
        return;
    }
    write ("\n");
    merkanz = min(max-count, merkanz);
    if (!count)
        write ( wrap (
        "Du hast "+max+" Rätsel gelöst und noch kein"
        " Merkmal von Dir beschrieben, daher kannst Du "+merkanz+
        " Merkmal"+(merkanz==1?"":"e")+" von Dir beschreiben."));
    else
        write ( wrap ("Du hast "+max+" Rätsel gelöst und erst "+count+
        " Merkmale von Dir beschrieben, daher kannst Du noch "+(merkanz)+
        " weitere"+(merkanz==1?"s":"")+" Merkmal"+(merkanz==1?"":"e")+
        " von Dir beschreiben.") +
        "\nDeine bisherigen Merkmale:\n");
    tmp = "\nFolgende Merkmale kannst Du für immer festlegen:\n";
    for (i=0; i < sizeof(merkmale); i++) {
        // fell und haut schliessen sich gegenseitig aus
        if ((i == INDEX(D_FELL)) && (sizeof(source) > INDEX(D_HAUT)) && SOURCE(D_HAUT)) continue;
        if ((i == INDEX(D_HAUT)) && (sizeof(source) > INDEX(D_FELL)) && SOURCE(D_FELL)) continue;
        if ((sizeof(source) > i) && source[i])
            write ("    "+merkmale[i][0]+": "+merkmale[i][source[i]]+"\n");
        else
           tmp += right(i+1,6)+": "+merkmale[i][0]+"\n";
    }
    write (tmp);
    write ("     0: Fertig\n\n");
    write ("Merkmalsnummer: ");
    input_to ("in_menu");
}


void in_menu (string s)
{
    int i, nr;
    int *source;

    source = this_player()->query_description_source();
    if ((s == "0") || (s == "q")) {
        write ("Okay, fertig.\n");
        return;
    }
    nr = to_int (s);
    if (nr>0 && (nr <= sizeof(merkmale))) {
        if (((nr-1 == INDEX(D_FELL)) && (sizeof(source) > INDEX(D_HAUT)) && SOURCE(D_HAUT))
         || ((nr-1 == INDEX(D_HAUT)) && (sizeof(source) > INDEX(D_FELL)) && SOURCE(D_FELL)))
            write ("Haut und Fell gleichzeitig geht nicht.\n");
        else if ((sizeof(source) >= nr) && source [nr-1])
            write ("Dieses Merkmal hast Du bereits festgelegt.\n");
        else {
            nr--;
            write ("\nSo, jetzt bitte Eigenschaft \""+merkmale[nr][0]+
                "\" wählen:\n");
            if ((nr == 0) || (nr == 2) || (nr == 8) || (nr == 9))
                for (i=1; i<sizeof(merkmale[nr]); i+=2) {
                    write (left(right (i,6)+": "+merkmale[nr][i],37));
                    if (i <sizeof(merkmale[nr])-1)
                        write (right (i+1,3)+": "+merkmale[nr][i+1]);
                    write ("\n");
                }
            else
                for (i=1; i<sizeof(merkmale[nr]); i++)
                    write (right (i,6)+": "+merkmale[nr][i]+"\n");
            write (right (0,6)+": Doch lieber nicht.\n");
            X[this_player()->query_real_name()] = nr+1;
            write ("Nummer der gewünschten Eigenschaft: ");
            input_to ("haben_will");
            return;
        }
    }
    if (s == "" || nr<0)
        input_to ("in_menu");
    else
        do_menu ();
}


void haben_will (string s)
{
    int nrm, nr;
    if (!(nrm = X[this_player()->query_real_name()])) {
        write ("Ups, da ist ein übler Fehler passiert.\n");
        return;
    }
    nrm--;
    if (!s || (s == "") || (s == "q") || (s == "0")) {
        write ("Na dann nicht...\n");
        do_menu ();
        return;
    }
    if (strlen (s) > 30) {
        write ("Ups. Das ist ein wenig zu lang.\nKürzere Eingabe: ");
        input_to ("haben_will");
        return;
    }
    if (strstr (s,",") != -1) {
        write ("Ups. Ein Komma passt da nicht rein.\nKommalose Eingabe: ");
        input_to ("haben_will");
        return;
    }
    nr = to_int (s);
    if (!nr || (nr >= sizeof(merkmale[nrm]))) {
        if (!nr) write ("Diese Zahl ist ungültig.\n");
        else write ("Diese Zahl ist zu groß.\n");
        do_menu ();
        return;
    }
    this_player()->set_one_description_source (nrm,nr);
    write ("Eigenschaft gesetzt.\n");
    do_menu ();
}

