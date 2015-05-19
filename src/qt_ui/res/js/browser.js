
var CBrowser = new function () {

    var CBrowser = this;
    this.showFullId = function (div, fullId) {
        div.find("a.text").html(fullId.substring(0, 25) + "...");
        div.find("li").find("a").css('display', 'inline-block');
    };
    this.hideFullId = function (div, fullId) {
        div.find("a.text").html(fullId.substring(0, 10) + "...");
        div.find("li").find("a").hide();
    };
    this.toggleFullId = function (div) {
        var fullId = div.find("a.text").attr("fullid");
        if (fullId.substring(0, 25) + "..." === div.find("a.text").html())
            this.hideFullId(div, fullId);
        else
            this.showFullId(div, fullId);
    };
    this.getImageFrJson = function (json) {
        try {
//            console.log(json.contentJsonB64[0]);
            var ctt = json.contentJsonB64[0];
            var k;
            for (k in ctt.content) {

                if (ctt.content[k].cc_name == "CC_FILE_CONTENT")
                    return ctt.content[k].content;
            }
        } catch (e) {
            console.log("getImageFrJson error");
        }

    };
    this.getImage = function (clink) {
        var cj = (BrowserAPI.getContentByLink(clink));
        r = this.getImageFrJson(cj);
        return '<img src="data:image/jpg;base64,' + r + '" class="brimg"/>';

    };
    this.toggleCmt = function (div) {
        var s = div.find("a.shrt").hasClass("short");
        if (s) {
            div.find("li").find("a").show();
            div.find("a.shrt").removeClass("short");
            
        } else {
            div.find("li").find("a").hide();
            div.find("a.shrt").addClass("short");
        }
    };

};

$(document).ready(function () {
    $("#test").click(function () {
//        alert("test!! " + BrowserAPI.createContent("tmp/tmp1.jpg"));
        console.log(BrowserAPI.getContentByLink("ccc:4310.1"));
    });
    $(".id").find("a.text").click(function () {
        CBrowser.toggleFullId($(this).parent());
    });
    $(".id-follow-btn").click(function () {
        alert("To Do");
    });
    $(".id-share-btn").click(function () {
        alert("To Do");
    });
    $(".brctt").find("a.shrt").click(function () {
        CBrowser.toggleCmt($(this).parent());
    });

    $("#test1").html(CBrowser.getImage("ccc:4310.1"));
});