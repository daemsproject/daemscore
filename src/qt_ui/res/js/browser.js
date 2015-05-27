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
    this.getFileContentFrJson = function (json) {
        try {
            var ctt = json.content[0];
            var k;
            for (k in ctt.content) {
                if (ctt.content[k].cc_name == "CC_FILE_CONTENT" || ctt.content[k].cc_name == "CC_FILE")
                    return ctt.content[k].content;
            }
        } catch (e) {
            console.log("getFileContentFrJson error");
        }

    };
    this.getImage = function (clink) {
        var cj = (BrowserAPI.getContentByLink(clink));
        r = this.getFileContentFrJson(cj);
        return this.createImgHtml(clink, r);

    };
    this.createImgHtml = function (clink, imgB64Data, type) {
        type = typeof type !== 'undefined' ? type : "image/jpeg";
        var idiv = $("#image-tpl").clone(true, true);
        idiv.find("img").attr("id", CLink.setString(clink).toHtmlId());
        idiv.find("img").attr("type", type);
        idiv.find("img").attr("src", "data:" + type + ";base64," + imgB64Data);
        return idiv.html();
    };
    this.createVideoHtml = function (clink, vdoB64Data, type) {
        type = typeof type !== 'undefined' ? type : "video/mp4";
        var vdiv = $("#video-tpl").clone(true, true);
        vdiv.find("video").attr("id", CLink.setString(clink).toHtmlId());
        vdiv.find("source").attr("type", type);
        vdiv.find("source").attr("src", "data:" + type + ";base64," + vdoB64Data);
        return vdiv.html();
    };
    this.createAudioHtml = function (clink, adoB64Data, type) {
        type = typeof type !== 'undefined' ? type : "audio/mpeg";
        var vdiv = $("#video-tpl").clone(true, true);
        vdiv.find("video").attr("id", CLink.setString(clink).toHtmlId());
        vdiv.find("source").attr("type", type);
        vdiv.find("source").attr("src", "data:" + type + ";base64," + adoB64Data);
        return vdiv.html();
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
    this.showFullImg = function (div) {
        if (div.find("img").length !== 0) {
            $("#fullImage").html(div.html());
            $("#fullImage").find("img").removeClass("brimg");
        }
    };
    this.toggleLink = function (div) {
        var cl = div.parent().parent().find(".linkspan");
        if (cl.html() === "")
            cl.html(cl.attr("clink"));
        else
            cl.html("");
    };

    this.shortenId = function (id) {
        return id.substr(0, 10) + "...";
    };
    this.addContent = function (ctt, fPos) {
        fPos = typeof fPos !== 'undefined' ? fPos : true;
        if (ctt.content[0].content === "non-standard")
            return;
        var sdiv = $("#standard").clone(true, true);
        sdiv.removeAttr("id");
        sdiv.find(".id").find(".text").attr("fullid", ctt.poster[0]);
        sdiv.find(".id").find(".text").html(this.shortenId(ctt.poster[0]));
        sdiv.find(".linkspan").attr("clink", ctt.link);

        if (this.isContentImage(ctt))
            sdiv.find(".ctt").html(this.createImgHtml(ctt.link, this.getFileContentFrJson(ctt)));
        else if (this.isContentVideo(ctt))
            sdiv.find(".ctt").html(this.createVideoHtml(ctt.link, this.getFileContentFrJson(ctt)));
        else if (this.isContentAudio(ctt))
            sdiv.find(".ctt").html(this.createAudioHtml(ctt.link, this.getFileContentFrJson(ctt)));
        else if (this.isContentText(ctt))
            sdiv.find(".ctt").html(atob(ctt.content[0].content[0].content));
        else
            return;

        fPos ? $("#mainframe").prepend(sdiv.children()) : $("#mainframe").append(sdiv.children());
//        console.log(ctt);
    };
    this.isContentImage = function (ctt) {
        return ctt.content[0].cc_name === "CC_FILE_P" &&
                ctt.content[0].content[1].cc_name === "CC_FILE_TYPESTRING" &&
                ctt.content[0].content[1].content === btoa("image/jpeg");
    };
    this.isContentVideo = function (ctt) {
        return ctt.content[0].cc_name === "CC_FILE_P" &&
                ctt.content[0].content[1].cc_name === "CC_FILE_TYPESTRING" &&
                ctt.content[0].content[1].content === btoa("video/mp4");
    };
    this.isContentAudio = function (ctt) {
        return ctt.content[0].cc_name === "CC_FILE_P" &&
                ctt.content[0].content[1].cc_name === "CC_FILE_TYPESTRING" &&
                ctt.content[0].content[1].content === btoa("audio/mpeg");
    };
    this.isContentText = function (ctt) {
        return ctt.content[0].cc_name === "CC_TEXT_P" &&
                ctt.content[0].content[0].cc_name === "CC_TEXT" &&
                ctt.content[0].content[0].content.length !== 0;
    };

    this.refreshNew = function () {
        var ctts = this.getRecentContents();
        for (k in ctts) {
            this.addContent(ctts[k], false);
        }
    };
    this.refreshOld = function () {
        var ctts = this.getOldContents();
        for (k in ctts) {
            this.addContent(ctts[k], false);
        }
    };
    this.getRecentContents = function () {
        var rc = 3;
        var lbh = BrowserAPI.getBlockCount();
        var bh = lbh;
        bh -= 10;
        var ctts = BrowserAPI.getRecent(bh, 10, false);
        bh -= 10;
        while (ctts.length < rc && bh > 0) {
            var tmp = BrowserAPI.getRecent(bh, 10, false);
            ctts = ctts.concat(tmp);
            bh -= 10;

        }
        blkDisp = [bh + 10, parseInt(lbh)];
        return ctts;
    };
    this.getOldContents = function () {
        var rc = 3;
        var lbh = blkDisp[0];
        var bh = lbh;
        bh -= 10;
        var ctts = BrowserAPI.getRecent(bh, 10, false);
        bh -= 10;
        while (ctts.length < rc && bh > 0) {
            var tmp = BrowserAPI.getRecent(bh, 10, false);
            ctts = ctts.concat(tmp);
            bh -= 10;

        }
        blkDisp = [bh + 10, lbh];
        return ctts;
    };

};
var blkDisp;
$(document).ready(function () {

    CBrowser.refreshNew();
    $("#refresh-btn").click(function () {
        CBrowser.refreshNew();
    });
    $("#refreshold-btn").click(function () {
        CBrowser.refreshOld();
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
    $(".ctt").click(function () {
        CBrowser.showFullImg($(this));
    });
    $(".ctt-link-btn").click(function () {
        CBrowser.toggleLink($(this));
    });


    $("#fullImage").click(function () {
        $(this).html("");
    });
    $("#test-btn").click(function () {
//        CBrowser.getRecentContents();
        var clink = CLink.setString("ccc:11175.1");
//        var clink2 = CLink.setString("11175.1");
        var clink2 = CLink.setString("11175.1.1");
        console.log(clink.toString());
        console.log(clink2.toHtmlId());
    });
    $(window).scroll(function () {
        if ($(window).scrollTop() + $(window).height() == $(document).height()) {
//            alert("bottom!");
            CBrowser.refreshOld();
        }
    });
});
