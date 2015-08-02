function numberWithCommas(a) {
    var b = a.toString().split(".");
    b[0] = b[0].replace(/\B(?=(\d{3})+(?!\d))/g, ",");
    return b.join(".")
}
(function(a) {
    a.fn.countTo = function(f) {
        f = a.extend({}, a.fn.countTo.defaults, f || {});
        var d = Math.ceil(f.speed / f.refreshInterval), b = (f.to - f.from) / d;
        var e = this, c = 0, h = f.from;
        if (e.interval) {
            clearInterval(e.interval);
            e.interval = null
        }
        e.target_value = f.to;
        e.interval = setInterval(g, f.refreshInterval);
        function g() {
            h += b;
            c++;
            e.original_value = h;
            a(e).html(formatSymbol(h, symbol_local));
            if (typeof (f.onUpdate) == "function") {
                f.onUpdate.call(e, h)
            }
            if (c >= d) {
                clearInterval(e.interval);
                e.interval = null;
                h = f.to;
                if (typeof (f.onComplete) == "function") {
                    f.onComplete.call(e, h)
                }
            }
        }}
    ;
    a.fn.countTo.defaults = {from: 0, to: 100, speed: 1000, refreshInterval: 200, decimals: 0, onUpdate: null, onComplete: null}
}(jQuery));
$(document).ready(function() {
    if (top.location != self.location) {
        top.location = self.location.href
    }
    try {
        $(".slidedeck").slidedeck()
    } catch (b) {
    }
    try {
        $("a[rel=popover]").popover({offset: 10}).click(function(c) {
            c.preventDefault()
        })
    } catch (b) {
    }
    setInterval(function() {
        if ($("#fade1").is(":visible")) {
            $("#fade1").hide();
            $("#fade2").fadeIn()
        } else {
            $("#fade1").fadeIn();
            $("#fade2").hide()
        }
    }, 5000);
    var a = $(".transacted:first-child");
    if (a.length > 0) {
        webSocketConnect(function(c) {
            c.onmessage = function(k) {
                var j = $.parseJSON(k.data);
                symbol = symbol_local;
                if (j.op == "utx") {
                    var d = TransactionFromJSON(j.x);
                    var l = 0;
                    for (var h = 0; h < d.out.length; h++) {
                        l += parseInt(d.out[h].value)
                    }
                    var m = parseInt(a.original_value);
                    if (!m || isNaN(m)) {
                        m = a.find("span").data("c")
                    }
                    var g = parseInt(a.target_value);
                    if (!isNaN(g) && g > m) {
                        l += g - m
                    }
                    var f = m + l;
                    a.countTo({from: m, to: f, speed: 10000, refreshInterval: 50})
                }
            };
            c.onopen = function() {
                c.send('{"op":"ip_sub", "ip":"127.0.0.1"}')
            }
        })
    }
    $("#youtube-preview").click(function() {
        $(this).empty().append('<iframe width="100%" height="256" src="https://www.youtube.com/embed/Um63OQz3bjo?autohide=1&controls=0&showinfo=0&autoplay=1" frameborder="0" allowfullscreen></iframe>')
    });
    setTimeout(function() {
        $("#pingit-youtube-preview").empty().append('<iframe width="100%" height="315" src="https://www.youtube.com/embed/dFgRm2ijqAM?autohide=1&controls=0&showinfo=0" frameborder="0" allowfullscreen></iframe>');
        $("#forgot-btn").click(function() {
            window.location = root + "wallet/forgot-identifier?param1=" + encodeURIComponent($("#forgot").val())
        });
        $("#download-instructions-btn").click(function() {
            $("#download-instructions").toggle(400)
        });
        if ($("#slideshow").length > 0) {
            var c = 0;
            var d = function() {
                ++c;
                if (c == 3) {
                    c = 0
                }
                $("#slideshow img").hide();
                $("#slideshow img").eq(c).fadeIn()
            };
            setInterval(d, 7000)
        }
    }, 500)
});