// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/room/rathaus/obj/drachenbaum.c
// Description: Ein wunderschoener Drachenbaum
// Author:	Anin

inherit "/i/item";
inherit "/i/move";

void create() {
    set_name("drachenbaum");
    set_gender("maennlich");
    set_id(({"baum","drachenbaum","pflanze","bäumchen","drachenbaeumchen"}));
    set_smell("Diese Pflanze scheint außer Erde, in der sie wächst, "
              "absolut nichts an wahrnehmbaren Gerüchen von sich zu geben.");
    set_noise("Sie ist so lange still, bis Du ihre Blätter in Bewegung "
              "versetzt, woraufhin sie eine Weile sanft raschelt.");
    set_feel("Vorsichtig fährst Du durch die abstehenden Blätter, die "
             "sanft über Dich hinwegstreichen.");
    set_long("Dein erster Eindruck ist: Buschige Mini-Palme. Das Bäumchen "
             "ist keinen Meter groß und hat einen Stamm, der eine gute "
             "Handbreit über der Erde in fünf Stämme übergeht, an deren "
             "oberen Enden viele lange, dünne Blätter ringsum abstehen. "
             "Es handelt sich also um einen glücklichen Drachenbaum, denn "
             "andernfalls würden die Blatter nicht so abstehen, sondern dicht "
             "am Stamm herunterhängen.");
    set_adjektiv(({"glücklich"}));
    set_material("pflanzlich");
    add_v_item(([
	 "name" : "stamm",
       "gender" : "maennlich",
	 "long" : "Der Stamm sieht ein wenig so aus, als wäre er aus flachen "
                  "Bändern geflochten, was durch die ehemaligen "
                  "Blattansätze kommt. Er ist zwar etwas holzig und "
                  "beige-braun, wirkt aber so, als hätte er sich noch nicht "
                  "weit vom Dasein als junges grünes Pflanzenteil entfernt. "
                  "Dazu passt auch, dass er, bevor er sich aufteilt, gerade mal "
                  "gut einen Daumen dick ist und das, was danach kommt, jeweils "
                  "nicht mal mehr die Dicke eines kleinen Fingers erreicht.",
	 "feel" : "Fühlt sich schuppig an. Man fühlt deutlich die "
                  "versetzten Kanten, an denen vor mehr oder weniger langer "
                  "Zeit noch Blätter angewachsen waren. Wenn man von unten "
                  "nach oben darüberfährt, merkt man noch, dass sich "
                  "zwischen den Kanten eine glatte Oberfläche befindet, in die "
                  "Gegenrichtung überwiegt eindeutig der Widerhaken-Effekt.",
	"smell" : "Der Stamm scheint keinerlei Geruch abzugeben.",
	   "id" : ({"stamm"}),
                ]));
    add_v_item(([
	 "name" : "blätter",
       "gender" : "saechlich",
       "plural" : 1,
	 "long" : "Die ausgewachsenen Blätter sind eine Elle lang und "
                  "höchstens so breit wie ein kleiner Finger, also sehr "
                  "lang und schmal. Sie sind dunkelgrün und haben einen "
                  "weinroten Rand. Sie sind direkt am Stamm angewachsen, den "
                  "sie an ihrem Ende versetzt jeweils eine gute halbe "
                  "Umdrehung einwickeln.",
     "adjektiv" : "länglich",
	 "feel" : "Die Oberseite fühlt sich wachsartig glatt an, wogegen die "
                  "Unterseite eher samtig glatt ist, wodurch man leichter und "
                  "ungebremster darüberfahren kann.",
	"smell" : "Die Blätter scheinen keinerlei Geruch abzugeben.",
	"noise" : "Ihks! Du hast mit einem der spitzen Blattenden Dein Ohr "
                  "getroffen! Du versuchst es nochmals mit mehr "
                  "Sicherheitsabstand und bemerkst, dass die Blätter "
                  "gemeinsam ein beruhigendes Rascheln erzeugen, solange sie "
                  "in Bewegung sind.",
     "hear_msg" : "$Der(OBJ_TP) halt $seinen(([name:ohr,gender:saechlich]),"
                  "ein,OBJ_TP) dicht an die Blätter des Drachenbaumes - und "
                  "schreckt zurück, als ein Blatt das Ohr mit der Spitze "
                  "trifft. Manchmal scheint Lauschen ein echtes Abenteuer zu "
                  "sein!",
	   "id" : ({"blatt","blätter"}),
                ]));
    add_v_item(([
	 "name" : "zweige",
       "gender" : "maennlich",
       "plural" : 1,
	 "long" : "Eigentlich hat ein Drachenbaum keine Zweige, sondern nur "
                  "einen Stamm. Allerdings können aus solch einem Stamm "
                  "mehrere neue Stämme sprießen, wenn er abbricht. Und "
                  "genau das wurde hier eine Handbreit über der Erde "
                  "fachmännisch provoziert. Daher sieht es so aus, als "
                  "hätte der Baum fünf Zweige.",
	 "feel" : "Fühlt sich schuppig an. Man fühlt deutlich die "
                  "versetzten Kanten, an denen vor mehr oder weniger langer "
                  "Zeit noch Blätter angewachsen waren. Wenn man von unten "
                  "nach oben darüberfährt, merkt man noch, dass sich "
                  "zwischen den Kanten eine glatte Oberfläche befindet, in die "
                  "Gegenrichtung überwiegt eindeutig der Widerhaken-Effekt.",
	"smell" : "Die Zweige scheinen keinerlei Geruch abzugeben.",
	   "id" : ({"zweige","zweig","äste","ast"}),
                ]));
    add_v_item(([
	 "name" : "wurzeln",
       "gender" : "weiblich",
       "plural" : 1,
	 "long" : "Du kannst nur den Wurzelansatz sehen, wo der eine Stamm "
                  "in ein Büschel runder, relativ glatter, holziger Wurzeln "
                  "übergeht.",
         "feel" : "Fühlt sich fast so an, als habe sich beim Gießen dort "
                  "Kalk abgelagert.",
        "smell" : "Du riechst deutlich die Erde, aber nichts von der Wurzel.",
	   "id" : ({"wurzel","wurzeln","wurzelansatz"}),
                ]));
    add_v_item(([
	 "name" : "erde",
       "gender" : "weiblich",
	 "long" : "Ganz normale Erde, in der dieses Bäumchen zuhause ist.",
         "feel" : "Fühlt sich etwas feucht an.",
        "smell" : "Riecht kräftig erdig und feucht.",
        "noise" : "Du meinst noch immer etwas Wasser darin versickern zu "
                  "hören.",
	   "id" : ({"erde"}),
                ]));
}
