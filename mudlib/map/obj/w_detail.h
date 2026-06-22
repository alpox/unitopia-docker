
  set_share_v_items(1);
  add_v_item(([
    "name"     : "schwanzfluke",
    "gender"   : "weiblich",
    "id"       : ({"fluke", "schwanzflosse","schwanzfluke",
                   "natural#weapon"}),
    "long"     : "Mit seiner beeindruckend großen Schwanzfluke bewegt "
                 "sich der Pottwal scheinbar spielend leicht durch das "
                 "Wasser.",
    "look_msg" : "$Der() betrachtet mit gebietender Ehrfurcht die "
                 "riesige Schwanzfluke des Pottwales."
    ]));

  add_v_item(([
    "name":"flossen",
    "gender":"weiblich",
    "plural":1,
    "id":({"flosse","flossen"}),
    "long":"Du kannst an dem Wal keine Flossen finden. Stattdessen "
           "benutzt er zur Fortbewegung die mächtige Schwanzfluke, "
           "sowie die flossenartigen Vordergliedmaße.",
    "look_msg":"$Der() sucht den Wal vergebens nach Flossen ab."
    ]));

  add_v_item(([
    "name":"vordergliedmaße",
    "gender":"saechlich",
    "plural":1,
    "id":({"vordergliedmaße","gliedmaße","seitenflosse"}),
    "long":"Die Vordergliedmaße sind zu flossenartigen Gebilden "
           "umgestaltet, die dem Wal zur Steuerung dienen.",
    "look_msg":"$Der() wirft einen schnellen Blick auf die momentan "
               "aus dem Wasser ragenden Vordergliedmaße."
    ]));

  add_v_item(([
    "name":"körper",
    "gender":"maennlich",
    "id":({"körper","walkörper"}),
    "long":"Von dem etwa 20 Meter messenden, gedrungenen Körper "
           "des Wales ist nur ein Teil der Oberseite und zuweilen "
           "auch die Schwanzfluke über der Wasseroberfläche sichtbar.",
    "look_msg":"$Der() versucht, den 20 Meter messenden Wal der "
               "Länge nach zu betrachten."
    ]));

  add_v_item(([
    "name":"kopf",
    "gender":"maennlich",
    "id":({"kopf","walkopf","vorderende","oberkopf","haupt"}),
    "long":"Ein eingelagertes Walratkissen verleiht dem massigen Kopf "
           "des Pottwales seine unförmige Gestalt. Der Oberkopf "
           "ist dadurch so stark aufgewölbt, dass das Vorderende "
           "nahezu senkrecht in die Höhe ragt. Weit vorne, an der "
           "Oberseite des mächtigen Walkopfes, befindet sich das "
           "Blasloch.", // *einen abbrech* - vielleicht gehts ja
                        // auch noch einfacher
    "look_msg":"$Der() bestaunt den massigen Walkopf."
    ]));

  add_v_item(([
    "name":"auge",
    "gender":"saechlich",
    "id":({"auge","augen"}),
    "long":"Die gegen den restlichen Körper geradezu winzig wirkenden "
           "Augen des Wales befinden sich zu beiden Seiten weit "
           "unten am überdimensionierten Kopf. Kurz dahinter setzen "
           "die kompakten, kurzen Vordergliedmaße an.",
    "look_msg":"$Der() schaut sich eines der eher kleinen Augen "
               "des Wales genauer an."
    ]));

  add_v_item(([
    "name":"haut",
    "gender":"weiblich",
    "id":({"haut","epidermis","oberfläche","natural#armour",
           "falten","hautfalten","farbe"}),
    "long":"Die glänzende Haut des Wales liegt an manchen Stellen "
           "in Falten und ist grau gefärbt.", 
    "look_msg":"$Der() lässt $seinen(([name:blick,gender:maennlich]),"
               "0,OBJ_TP) über die Haut des Wales streifen."
    ]));

  add_v_item(([
    "name":"blasloch",
    "gender":"saechlich",
    "id":({"blasloch","loch","atemwegsoeffnung","atemloch",
           "atemöffnung"}),
    "long":"Weit vorne auf der Oberseite des mächtig aufgewölbten "
           "Walkopfes kann man das Blasloch erkennen, welches als "
           "Atemöffnung dient.",
    "look_msg":"$Dem() gelingt es, einen Blick auf das Blasloch "
               "zu werfen."
    ]));

  add_v_item(([
    "name":"maul",
    "gender":"saechlich",
    "id":({"maul","mund","mundöffnung"}),
    "long":"Das Maul des Wales erstreckt sich bis zu den "
           "kleinen Augen und trennt den voluminösen oberen "
           "Teil des Kopfes von dem vergleichsweise flachen "
           "und schmalen bezahnten Unterkiefer.",
    "look_msg":"$Der() betrachtet das riesige Maul des Wales."
    ]));

  add_v_item(([
    "name":"zähne",
    "gender":"maennlich",
    "plural":1,
    "id":({"zähne","zahn","walzähne","walzahn"}),
    "long":"Wenn der Wal sein Maul öffnet, kann man eine Reihe, auf "
           "dem Unterkiefer sitzender, weißer, spitzer Zähne erkennen.",
    "look_msg":"$Der() versucht, dem Wal ins geöffnete Maul zu blicken."
    ]));

  add_v_item(([
    "name":"unterkiefer",
    "gender":"maennlich",
    "id":({"unterkiefer","kiefer"}),
    "long":"Der schmale Unterkiefer des Wales verschwindet regelrecht "
           "unter dem riesigen Oberkopf. Er trägt eine Reihe kleiner, "
           "spitzer Zähne.",
    "look_msg":"$Der() mustert den unteren Teil des Walkopfes."
    ]));

  add_v_item(([
    "name":"bauch",
    "gender":"maennlich",
    "id":({"bauch","unterseite","bauchseite","dorsalseite"}),
    "long":"Der Bauch des Wales ist ständig im Wasser und daher "
           "nur sehr verschwommen zu erkennen.",
    "look_msg":"$Der() versucht vergeblich, den Bauch des Wales "
               "zu Gesicht zu bekommen."
    ]));

  add_v_item(([
    "name":"rücken",
    "gender":"maennlich",
    "id":({"rücken","rückseite","dorsalseite","walrücken"}),
    "long":"Der Walrücken ist kaum gebogen und geht mit einen "
           "leichten Knick in den Bereich des Schwanzes über.",
    "look_msg":"$Der() schaut den Walrücken entlang."
    ]));
  
  add_v_item(([
    "name":"schwanz",
    "gender":"maennlich",
    "id":({"schwanz","walschwanz"}),
    "long":"Der Schwanz des Wales fällt auf der Oberseite durch "
           "hervorstehende Wirbel auf, die den Übergang zum Rücken "
           "wellig erscheinen lassen. " // Hmmm..., gewagte Formulierung :)
           "Die waagerecht stehende Schwanzfluke bildet den Abschluss des "
           "Schwanzes, der etwas schmaler ist als der kompakte Körper.",
    "look_msg":"$Der() verfolgt den Schwanz des Wales mit $seinem"
               "(([name:augen,gender:saechlich,plural:1]),0,OBJ_TP)"
    ]));

  add_v_item(([
    "name":"blaswolke",
    "gender":"weiblich",
    "id":({"blaswolke","wolke"}),
    "long":"Kurz nachdem der Wal ausgeatmet hat, ist über ihm eine "
           "Blaswolke zu sehen, welche er mit hohem Druck aus seinen "
           "Atemwegen schleudert. Sie ist etwa sieben Meter hoch und "
           "weist schräg nach vorne.",
    "look_msg":"$Der() schaut nach oben."
    ]));
  set_share_v_items(0);
