function findpattern(text, pattern)
  local match = { text:find(pattern) }
  if # match > 2 then
    return match[3]
  else
    return ""
  end
end

local jsDocUrl = "http://flusspferd.org/tmp/js/"
function do_uc_first(a)
   return jsDocUrl .. a:sub(1,1):upper() .. a:sub(2)
end


function do_uc_word(prefix)
  return function(a)
    local uc_first = function(w)
      return w:sub(2,2):upper() .. w:sub(3)
    end

    print("here " ..a)
    a = uc_first("_" .. a):gsub('(_.)', uc_first)
    return jsDocUrl .. prefix .. a
  end
end


-- Rewrite rules for NaturalDocs -> JsDoc
local docMapping = {
  ["^/tmp/docs/files/src/spidermonkey/(.+)-jsdoc.html$"] = do_uc_first,
  ["^/tmp/docs/files/src/io/(.+)-jsdoc.html$"] = do_uc_word('IO.'),
  ["^/tmp/docs/files/plugins/.+/(.+)-js.html$"] = jsDocUrl,
  ["^/tmp/docs/files/plugins/.+/(.+)-jsdoc.html"] = do_uc_word(''),
  ["^/tmp/docs/index/(.*)"] = function() return jsDocUrl end
}

local uri = lighty.env["uri.path"] 
for k,v in pairs(docMapping) do 
  local match = findpattern(uri,k); 
   
  if match:len() > 0 then 
    if type(v) == "function" then 
      uri = v(match) 
    else 
      uri = v .. match 
    end 
    lighty.header["Location"] = uri 
    return 301 
  end 
end  

