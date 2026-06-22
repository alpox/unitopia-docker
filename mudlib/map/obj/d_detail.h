// Vielleicht noch ein Befuehlen einiger Stellen moeglich machen -
// schliesslich kann man ihn ja anfassen
//
//  Sorcerer   16.02.2012 - v_item 'stirn' hinzugefuegt

  set_share_v_items(1);
  add_v_item(([
    "name"    : "schnauze",
    "gender"  : "weiblich",
    "id"      : ({"schnauze","natural#weapon"}),
    "long"    : "Gezielte Stöße mit dieser spitzen Schnauze "
                "treiben selbst einen großen Hai in die Flucht.",
    "look_msg": "$Der() betrachtet die längliche Schnauze "
                "des Delphines."
    ]));

  add_v_item(([
    "name":"rückenfinne",
    "gender":"weiblich",
    "id":({"finne","rückenfinne"}),
    "long":"Die Rückenfinne erscheint spitz und ragt steil vom "
           "gebogenen Rücken auf. Sie dient dem Delphin zur "
           "Stabilisierung seiner Position im Wasser.",
           // *belehr* ;)
    "look_msg":"$Der() mustert die steil aufragende Rückenfinne "
               "des Delphines."
    ]));

  add_v_item(([
    "name":"schwanzfluke",
    "gender":"weiblich",
    "id":({"schwanzfluke","fluke"}),
    "long":"Die schmale Schwanzfluke setzt an der Schwanzwurzel des "
           "Delphines an und ragt zu beiden Seiten vom Körper weg. ",
    "look_msg":"$Der() versucht einen Blick auf die meist "
               "untergetauchte Schwanzfluke zu werfen."
    ]));

  add_v_item(([
    "name":"flosse",
    "gender":"weiblich",
    "id":({"flosse","flossen"}),
    "long":"Flossen besitzt der Delphin zwar nicht, aber deren "
           "Funktionen werden von der Rückenfinne, der Schwanzfluke "
           "sowie von den Flippern übernommen.",
    "look_msg":"$Der() sucht den Delphin vergeblich nach Flossen ab."
    ]));

  add_v_item(([
    "name":"schwanz",
    "gender":"maennlich",
    "id":({"schwanz","schwanzwurzel","hinterende"}),
    "long":"Der Schwanz ist oval geformt, so dass er das Wasser "
           "fast ohne Vortrieb zu durchschneiden vermag.",
    "look_msg":"$Der() betrachtet den Delphinschwanz."
    ]));

  add_v_item(([
    "name":"flipper",
    "gender":"weiblich",
    "plural":1,
    "id":({"vordergliedmaße","gliedmaße","seitenflosse",
           "flipper"}),
    "long":"Die Flipper des Delphines weisen aufgrund der "
           "Anpassung an das Wasser eine kompakte, plattenförmige "
           "Struktur auf. Sie erinnern in ihrer Form an Pinguinflügel "
           "und dienen vorwiegend der Steuerung.",
    "look_msg":"$Der() mustert die Flipper des Delphines."
    ]));

  add_v_item(([
    "name":"haut",
    "gender":"weiblich",
    "id":({"haut","oberfläche","delphinhaut","epidermis",
           "farbe","hautfarbe","natural#armour"}),
    "long":"Die glänzende Delphinhaut ist bläulich-grau gefärbt und "
           "sieht sehr glatt aus. Sie setzt dem Wasser auch bei hohen "
           "Geschwindigkeiten nur wenig Reibungswiderstand entgegen.",
    "look_msg":"$Der() mustert die glatte Delphinhaut.",
    "feel":"Die nasse Delphinhaut fühlt sich an, als "
           "bestünde sie aus besonders glattem Gummi.",
    "feel_msg":"$Der() betastet die Delphinhaut."       
    ]));

  add_v_item(([
    "name":"kopf",
    "gender":"maennlich",
    "id":({"kopf","vorderende"}),
    "long":"Der Delphinkopf schließt sich nahtlos an den übrigen "
           "Körper an und wird zur Schnauze hin zunehmend schmaler. "
           "Seitlich des Kopfes befinden sich die kleinen Augen und "
           "dazwischen, auf der Stirn, kann man das Blasloch erkennen.",
    "look_msg":"$Der() schaut sich interessiert den Kopf des "
               "Delphines an."
    ]));

  add_v_item(([
    "name":"körper",
    "gender":"maennlich",
    "id":({"körper","delphinkoerper"}),
    "long":"Der schlanke Körper des Delphines ist fast ständig in "
           "Bewegung. Manchmal hält er jedoch inne und richtet sich auf, "
           "wobei er nur noch mit dem Schwanz ins Wasser eintaucht.",
    "look_msg":"$Der() versucht, den Delphin der ganzen Länge nach zu "
               "mustern."
    ]));

  add_v_item(([
    "name":"augen",
    "gender":"saechlich",
    "plural":1,
    "id":({"augen","auge","äugelchen","äuglein"}),
    "long":"Die winzigen Augen befinden sich seitlich am Delphinkopf, "
           "knapp hinter den langgezogenen Mundwinkeln.",
    "look_msg":"$Der() schaut dem Delphin in die Augen."
    ]));

  add_v_item(([
    "name":"maul",
    "gender":"saechlich",
    "id":({"maul","mund","mundöffnung","mundwinkel"}),
    "long":"Der Mund des Delphines reicht von der Schnauze "
           "bis unter die Augen.",
    "look_msg":"$Der() bestaunt den breiten Mund des Delphines."
    ]));

  add_v_item(([
    "name":"bauch",
    "gender":"maennlich",
    "id":({"bauch","unterseite","ventralseite"}),
    "long":"Der Bauch des Delphines ist viel heller gefärbt "
           "als der Rest des Körpers.",
    "look_msg":"$Der() versucht einen Blick auf die Unterseite "
               "des Delphines zu werfen."
    ]));

  add_v_item(([
    "name":"rücken",
    "gender":"maennlich",
    "id":({"rücken","rückseite","oberseite","dorsalseite"}),
    "long":"Der Rücken ist der am intensivsten gefärbte "
           "Bereich des Delphinkörpers. Etwa in der Mitte "
           "kann man die Rückenfinne erkennen.",
    "look_msg":"$Der() mustert den Rücken des Delphines."
    ]));

  add_v_item(([
    "name":"zähne",
    "gender":"maennlich",
    "plural":1,
    "id":({"zähne","zahn","zähnchen","zahnreihe","gebiss","kiefer"}),
    "long":"Wenn der Delphin seinen Mund öffnet, sind am Innenrand "
           "der Schnauze eine Reihe spitzer, weißer Zähne zu sehen.",
    "look_msg":"$Der() schaut dem Delphin ins geöffnete Maul."
    ]));

  add_v_item(([
    "name":"blasloch",
    "gender":"saechlich",
    "id":({"blasloch","loch","atemausgang"}),
    "long":"Auf der Oberseite zwischen den Augen befindet sich das "
           "Blasloch des Delphines, mit dessen Hilfe er an der Luft "
           "gewaltige Atemzüge machen kann.",
    "look_msg":"$Der() untersucht interessiert das Blasloch des "
               "Delphines."
    ]));

  add_v_item(([
    "name":"stirn",
    "gender":"weiblich",
    "id":({"stirn"}),
    "long":"Mitten auf der Stirn sitzt das Blasloch des Delphines. "
           "Mit dessen Hilfe kann er an der Luft atmen.",
    "look_msg":"$Der() betrachtet die Stirn des Delphines genauer."
    ]));
  set_share_v_items(0);
