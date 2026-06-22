// This file is part of UNItopia Mudlib.
// ----------------------------------------------------------------
// File:	/secure/master/handle_light.inc
// Description: Lichtbeschreibung von Reaumen
// Author:	Francis

#pragma strict_types

/*
 * Hier fragen die Raeume nach ihrem Tageszeit-String.
 */

static mixed *light_descriptions;

private void get_light_descriptions() {
    light_descriptions = ({
	({      0, "Es ist Geisterstunde."			}),
	({  10000, "Es ist Nacht."				}),
	({  40000, "Der Horizont hellt sich langsam auf."	}),
	({  42000, "Es d\u00e4mmert."				}),
	({  45000, "Die Sonne geht auf."			}),
	({  51000, "Es ist kurz nach Sonnenaufgang."		}),
	({  54000, "Es ist fr\u00fch am Morgen."		}),
	({  62000, "Es ist fr\u00fcher Vormittag."		}),
	({  73000, "Es ist vormittags."				}),
	({  90000, "Es ist sp\u00e4t am Vormittag."		}),
	({ 104000, "Es ist kurz vor Mittag."			}),
	({ 113000, "Die Sonne steht im Zenit."			}),
	({ 130000, "Es ist fr\u00fcher Nachmittag."		}),
	({ 143000, "Es ist nachmittags."			}),
	({ 161000, "Es ist sp\u00e4t am Nachmittag."		}),
	({ 174000, "Es ist fr\u00fch am Abend."			}),
	({ 182000, "Es ist abends."				}),
	({ 190000, "Es ist sp\u00e4t abends."			}),
	({ 193000, "Es ist kurz vor Sonnenuntergang."		}),
	({ 195000, "Die Sonne geht unter."			}),
	({ 201000, "Es d\u00e4mmert."				}),
	({ 205000, "Es wird dunkel."				}),
	({ 212000, "Es ist Nacht."				}),
	({ 240001, "Es ist Mitternacht."			})
    });
}

string query_light_description(int time) {
    int a;

    for (a=1; a<sizeof(light_descriptions); a++)
	if (time < light_descriptions[a][0])
	    return light_descriptions[a-1][1];
    return "Es ist irgendwann Tags\u00fcber.";
}
