var newDisp = []; // array to host block range shown on new page
var fllDisp = []; // follow page
var mypDisp = []; // mypage
var sldMax = 100;
var sld = [];
$(document).ready(function () {

    CBrowser.newAction();

    $("#refresh-btn").click(function () {
        CBrowser.refreshNew();
    });
    $("#refreshold-btn").click(function () {
        CBrowser.refreshOld();
    });
    $(".tabbar").children("li").children("a.ntcbtn").click(function () {
        CBrowser.switchTab($(this).attr("id"));
    });
    $("br-lang-btn").click(function () {
    });
    $(".linkspan").click(function () {
        copyToClipboard($(this).find("a").html());
    });
    $(".ctt-link-btn").click(function () {
        var link = $(this).parent().parent().find(".linkspan").attr("clink");
        copyToClipboard(link);
    });
//    $("#fullImage").click(function () {
//        $(this).html("");
//    });
    $(window).scroll(function () {
        if ($(window).scrollTop() + $(window).height() == $(document).height()) {
            CBrowser.bottomAction();
        }
    });

    var imgs = CBrowser.getNewImages();
    CBrowser.addSlideImage(imgs);
    function resetC2() {
        var c1w = $("#slider ul li").width() + $("body").width() * 0.1;
        $(".column2").css({marginLeft: c1w});
    }
    resetC2();
    $(window).resize(function () {
        resetC2();
    });

// slider code from http://codepen.io/zuraizm/pen/vGDHl
    var sliderInterval = 3000;
    var sliderAuto = setInterval(function () {
        moveRight();
    }, sliderInterval);

    var slideCount = $('#slider ul li').length;
    var slideWidth = $('#slider ul li').width();
    var slideHeight = $('#slider ul li').height();
    var sliderUlWidth = slideCount * slideWidth;
    $('#slider').css({width: slideWidth, height: slideHeight});
    $('#slider ul').css({width: sliderUlWidth, marginLeft: -slideWidth});
    $('#slider ul li:last-child').prependTo('#slider ul');
    $('#slider').mouseover(function () {
        clearInterval(sliderAuto);
    }).mouseleave(function () {
        sliderAuto = setInterval(function () {
            moveRight();
        }, sliderInterval);
    });
    function moveLeft() {
        $('#slider ul').animate({
            left: +slideWidth
        }, 200, function () {
            $('#slider ul li:last-child').prependTo('#slider ul');
            $('#slider ul').css('left', '');
        });
    }
    function moveRight() {
        $('#slider ul').animate({
            left: -slideWidth
        }, 200, function () {
            $('#slider ul li:first-child').appendTo('#slider ul');
            $('#slider ul').css('left', '');
        });
    }
    $('a.control_prev').click(function () {
        moveLeft();
    });
    $('a.control_next').click(function () {
        moveRight();
    });

// copy from publisherpage.js
    var dropZone = document.getElementById('fileholder');
    var theTextVal = "Type some here or drag and drop a file / Click or drag the file into the area above";
//    $('#fileholder').click(function () {
//        CPublisher.handleFileInput('theFile');
//    })
    $('#cblc').html(BrowserAPI.getBlockCount());
    $('#browsefile').click(function () {
        CPublisher.handleFileInput('theFile');
    })
    $('#theText').val(theTextVal).click(function () {
//        console.log($(this).val());
        $(this).removeClass('lttext');
        if ($(this).val() === theTextVal) {
            $(this).val("").addClass("ltborder").removeClass("noborder");
            $('#pubbtnh').show();
        }
    }).blur(function () {
        if ($(this).val() === "") {
            $(this).addClass('lttext').val(theTextVal).addClass("noborder").removeClass("ltborder");
        }
    });
    $('#addtag').click(function () {
//        alert('t');
        CPublisher.addTagField();
    });
    $('#addlink').click(function () {
        CPublisher.addLinkField();
        $(this).attr('disabled', true);
    });
    if (window.File && window.FileReader && window.FileList && window.Blob) {
        dropZone.addEventListener('dragover', CPublisher.handleDragOver, false);
        dropZone.addEventListener('drop', CPublisher.handleFileSelect, false);
    } else {
        alert("error handleDragOver");
    }
    $("#test-btn").click(function () {
        var t = "test text<br><br><br><br><br><br><br>test";
        CBrowser.showNotice(t, 100);
    });
    $("#theText").bind('input propertychange', function () {
        $("#confirmpub").removeAttr('disabled');
    });
    $("#confirmpub").click(function () {
        var text = $("#theText").val();
        console.log(text);
        var ctt = CPublisher.createTextContent(text);
        console.log(ctt);

        CPublisher.handleContent(ctt);
    });
    doTranslate();
    $('#langmenu li a').click(function () {
        langCodeF = $(this).attr("tr");
        console.log(langCodeF);
        langCode = langCodeF.substring(0, 2);
        doTranslate();
    });
});