var LangTextMyPoint = {
  en: {
    my_point: "my point",
    orca_list_thirdmodel: "3D Model Hub",
    image_generate_3d: "Image Generate 3D",
  },
  ca_ES: {
    my_point: "Benvingut a Orca-Flashforge",
  },
  es_ES: {
    my_point: "Bienvenido a Orca-Flashforge",
  },
  de_DE: {
    my_point: "Willkommen im Orca-Flashforge",
  },
  cs_CZ: {
    my_point: "Vítejte v Orca-Flashforge",
  },
  fr_FR: {
    my_point: "Bienvenue sur Orca-Flashforge",
  },
  zh_CN: {
    my_point: "我的积分",
    orca_list_thirdmodel: "三方模型库",
    image_generate_3d: "图生3D",
  },
  zh_TW: {
    my_point: "歡迎使用 Orca-Flashforge",
  },
  ru_RU: {
    my_point: "Приветствуем в Orca-Flashforge!",
  },
  ko_KR: {
    my_point: "Orca-Flashforge에 오신 것을 환영합니다",
  },
  tr_TR: {
    my_point: "Orca-Flashforge'a hoş geldiniz",
  },
  pl_PL: {
    my_point: "Witamy w Orca-Flashforge",
  },
  pt_BR: {
    my_point: "Bem-vindo ao Orca-Flashforge",
  },
  lt_LT: {
    my_point: "Sveikiname prisijungus prie Orca-Flashforge",
  },
};

var LANG_COOKIE_NAME = "BambuWebLang";
var LANG_COOKIE_EXPIRESECOND = 365 * 86400;

function TranslatePageMyPoint() {
  let strLang = GetQueryString("lang");
  if (strLang != null) {
    //setCookie(LANG_COOKIE_NAME,strLang,LANG_COOKIE_EXPIRESECOND,'/');
    localStorage.setItem(LANG_COOKIE_NAME, strLang);
  } else {
    //strLang=getCookie(LANG_COOKIE_NAME);
    strLang = localStorage.getItem(LANG_COOKIE_NAME);
  }

  //alert(strLang);

  if (!LangTextMyPoint.hasOwnProperty(strLang)) strLang = "en";

  let AllNode = $(".trans");
  let nTotal = AllNode.length;
  for (let n = 0; n < nTotal; n++) {
    let OneNode = AllNode[n];

    let tid = $(OneNode).attr("tid");
    if (LangTextMyPoint[strLang].hasOwnProperty(tid)) {
      $(OneNode).html(LangTextMyPoint[strLang][tid]);
    }
  }
}
