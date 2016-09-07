import kudzu
import string
from simpleconfig import SimpleConfigFile

class Keyboard (SimpleConfigFile):
    # the *first* item in each of these wins conflicts
    console2xlist = [
            ("be-latin1"         , ('pc102', 'be')),
            ("be2-latin1"        , ('pc102', 'be')),
            ("bg"                , ('pc102', 'bg')),
            ("cf"                , ('pc102', 'cf')),
            ("cz-lat2"           , ('pc102', 'cs')),
            ("cz-lat2-prog"      , ('pc102', 'cs')),
            ("de-latin1"         , ('pc102', 'de')),
            ("de"                , ('pc102', 'de')),
            ("de-latin1-nodeadkeys" , ('pc102', 'de')),
            ("dk"                , ('pc102', 'dk')),
            ("dk-latin1"         , ('pc102', 'dk')),
            ("es"                , ('pc102', 'es')),
            ("fi"                , ('pc102', 'fi')),
            ("fi-latin1"         , ('pc102', 'fi')),
            ("fr"                , ('pc102', 'fr')),
            ("fr-latin0"         , ('pc102', 'fr')),
            ("fr-latin1"         , ('pc102', 'fr')),
            ("fr-pc"             , ('pc102', 'fr')),
            ("fr_CH"             , ('pc102', 'fr_CH')),
            ("fr_CH-latin1"      , ('pc102', 'fr_CH')),
            ("hu"                , ('pc102', 'fr_CH')),
            ("hu101"             , ('pc102', 'hu')),
            ("it"                , ('pc102', 'it')),
            ("it-ibm"            , ('pc101', 'it')),
            ("it2"               , ('pc102', 'it')),
            ("jp106"             , ('jp106', 'jp')),
            ("no-latin1"         , ('pc102', 'no')),
            ("no"                , ('pc102', 'no')),
            ("pl"                , ('pc102', 'pl')),
            ("pt-latin1"         , ('pc102', 'pt')),
            ("ru"                , ('pc102', 'ru')),
            ("ru-cp1251"         , ('pc102', 'ru')),
            ("ru-ms"             , ('microsoft', 'ru')),
            ("ru1"               , ('pc102', 'ru')),
            ("ru2"               , ('pc102', 'ru')),
            ("ru_win"            , ('pc105', 'ru')),
            ("se-latin1"         , ('pc102', 'se')),
            ("uk"                , ('pc101', 'gb')),
            ("us"                , ('pc101', 'us')),
            ]

    console2xsun = {
	    "sun-pl-altgraph"	: 'pl',
	    "sun-pl"		: 'pl',
	    "sunt4-es"		: 'es',
	    "sunt5-cz-us"	: 'cs',
	    "sunt5-de-latin1"	: 'de',
	    "sunt5-es"		: 'es',
	    "sunt5-fi-latin1"	: 'fi',
	    "sunt5-fr-latin1"	: 'fr',
	    "sunt5-ru"		: 'ru',
	    "sunt5-uk"		: 'en_US',
	    "sunt5-us-cz"	: 'cs',
	    }

    console2x = {}
    for (console, (map, layout)) in console2xlist:
	console2x[console] = (map, layout)

    x2console = {}
    for (console, (map, layout)) in console2xlist:
	if not x2console.has_key((map, layout)):
	    x2console [(map, layout)] = console

    xsun2console = {}
    for (key, value) in console2xsun.items():
        xsun2console [value] = key
    
    
    def __init__ (self):
	self.type = "PC"
	self.model = None
	self.layout = None
        self.info = {}
	list = kudzu.probe(kudzu.CLASS_KEYBOARD, kudzu.BUS_UNSPEC,
			   kudzu.PROBE_ONE)
	if list:
	    (device, module, desc) = list[0]
	    if desc[:14] == 'Serial console':
		self.type = "Serial"
	    elif desc[:8] == 'Sun Type':
		self.type = "Sun"
		if desc[8:9] == '4':
		    self.model = 'type4'
		    desc = desc[10:]
		elif desc[8:14] == '5 Euro':
		    self.model = 'type5_euro'
		    desc = desc[15:]
		elif desc[8:14] == '5 Unix':
		    self.model = 'type5_unix'
		    desc = desc[15:]
		else:
		    self.model = 'type5'
		    desc = desc[10:]
		if desc[:8] == 'Keyboard':
		    self.layout = 'us'
		else:
		    xx = string.split (desc)
		    if xx[0] == 'fr_BE':
			self.layout = 'be'
		    elif xx[0] == 'fr_CA':
			self.layout = 'fr'
		    elif xx[0] == 'nl' or xx[0] == 'ko' or xx[0] == 'tw':
			self.layout = 'us'
		    else:
			self.layout = xx[0]
	if self.type == "Sun":
	    self.info["KEYBOARDTYPE"] = "sun"
	elif self.type != "Serial":
	    self.info["KEYBOARDTYPE"] = "pc"

    def available (self):
	if self.type == "Sun":
	    return [
		"sun-pl-altgraph",
		"sun-pl",
		"sundvorak",
		"sunkeymap",
		"sunt4-es",
		"sunt4-no-latin1",
		"sunt5-cz-us",
		"sunt5-de-latin1",
		"sunt5-es",
		"sunt5-fi-latin1",
		"sunt5-fr-latin1",
		"sunt5-ru",
		"sunt5-uk",
		"sunt5-us-cz",
	    ]
	if self.type == "Serial":
	    return [ "us" ]
        return [
            "azerty",
            "be-latin1",
            "be2-latin1",
            "fr-latin0",
            "fr-latin1",
            "fr-pc",
            "fr",
            "wangbe",
            "ANSI-dvorak",
            "dvorak-l",
            "dvorak-r",
            "dvorak",
            "pc-dvorak-latin1",
            "tr_f-latin5",
            "trf",
            "bg",
            "cf",
            "cz-lat2-prog",
            "cz-lat2",
            "defkeymap",
            "defkeymap_V1.0",
            "dk-latin1",
            "dk",
            "emacs",
            "emacs2",
            "es",
            "fi-latin1",
            "fi",
            "gr-pc",
            "gr",
            "hebrew",
            "hu101",
            "is-latin1",
            "it-ibm",
            "it",
            "it2",
            "jp106",
            "la-latin1",
            "lt",
            "lt.l4",
            "nl",
            "no-latin1",
            "no",
            "pc110",
            "pl",
            "pt-latin1",
            "pt-old",
            "ro",
            "ru-cp1251",
            "ru-ms",
            "ru-yawerty",
            "ru",
            "ru1",
            "ru2",
            "ru_win",
            "se-latin1",
            "sk-prog-qwerty",
            "sk-prog",
            "sk-qwerty",
            "tr_q-latin5",
            "tralt",
            "trf",
            "trq",
            "ua",
            "uk",
            "us",
            "croat",
            "cz-us-qwertz",
            "de-latin1-nodeadkeys",
            "de-latin1",
            "de",
            "fr_CH-latin1",
            "fr_CH",
            "hu",
            "sg-latin1-lk450",
            "sg-latin1",
            "sg",
            "sk-prog-qwertz",
            "sk-qwertz",
            "slovene",
            ]

    def set (self, keytable):
	if self.type != "Serial":
	    self.info["KEYTABLE"] = keytable

    def setfromx (self, model, layout):
	if self.type == "PC":
            mapping = Keyboard.x2console
        else:
            mapping = Keyboard.xsun2console
            
        keys = mapping.keys ()

        fuzzy = None
        for key in keys:
            if type(key) == type(()):
                (mod, lay) = key
            else:
                mod = model
                lay = key;
            if model == mod and layout == lay:
                self.info["KEYTABLE"] = mapping[key]
                return
            if layout == lay:
                fuzzy = key

        if fuzzy:
            self.info["KEYTABLE"] = mapping[fuzzy]
        else:
            if self.type == "PC":
                self.info["KEYTABLE"] = "us"
            else:
                return "sunkeymap"

    def get (self):
        if self.info.has_key ("KEYTABLE"):
            return self.info["KEYTABLE"]
        else:
	    if self.type == "Sun":
		for map in Keyboard.console2xsun.keys():
		    if Keyboard.console2xsun[map] == self.layout:
			return map
		return "sunkeymap"
	    else:
		return "us"

    def getXKB (self):
	if self.type == "PC":
	    if Keyboard.console2x.has_key (self.get ()):
		(model, keylayout) = Keyboard.console2x[self.get ()]
		return ("xfree86", model, keylayout, "", "")
	else:
	    if Keyboard.console2xsun.has_key (self.get ()):
		keylayout = Keyboard.console2xsun[self.get ()]
		return ("sun", self.model, keylayout, "", "")
