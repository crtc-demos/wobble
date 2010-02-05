type point = {
  x : int;
  y : int;
  mutable c : int
}

let xsize = 640
and ysize = 480

let random_points () =
  let pts = Array.create 64 { x = 0; y = 0; c = 0 } in
  for i = 0 to 63 do
    pts.(i) <- {
      x = Random.int xsize;
      y = Random.int ysize;
      c = Graphics.rgb (Random.int 256) (Random.int 256) (Random.int 256)
    }
  done;
  pts

let closest pts x y =
  let found = ref None
  and closest = ref max_int in
  for i = 0 to 63 do
    let pt = pts.(i) in
    let sqdist = (pt.y - y) * (pt.y - y)
                 + (pt.x - x) * (pt.x - x) in
    if sqdist < !closest then begin
      found := Some i;
      closest := sqdist
    end
  done;
  match !found with
    None -> raise Not_found
  | Some x -> x

exception Found of int

let slow_scan pts num scanline ~left =
  try
    begin
      if left then begin
	for x = 0 to xsize - 1 do
	  if closest pts x scanline = num then
            raise (Found x)
	done
      end else begin
	for x = xsize - 1 downto 0 do
          if closest pts x scanline = num then
	    raise (Found x)
	done
      end
    end;
    None
  with Found x -> Some x

let find_endpoint pts num scanline ~left =
  let l, r =
    if left then
      0, pts.(num).x
    else
      pts.(num).x, xsize - 1 in
  let rec scan lhs lhspt rhs rhspt =
    let mid = (lhs + rhs) / 2 in
    let midpt = closest pts mid scanline in
    if abs (lhs - rhs) < 2 then
      Some mid
    else begin
      match lhspt = num, midpt = num, rhspt = num with
        true, true, false ->
	  if left then
	    Some lhs
	  else
	    scan mid midpt rhs rhspt
      | true, false, false ->
          if left then
	    Some lhs
	  else
	    scan lhs lhspt mid midpt
      | false, true, true ->
          if not left then
	    Some rhs
	  else
	    scan lhs lhspt mid midpt
      | false, false, true ->
          if not left then
	    Some rhs
	  else
	    scan mid midpt rhs rhspt
      | true, true, true -> if left then Some lhs else Some rhs
      | _ -> slow_scan pts num scanline ~left
    end in
  scan l (closest pts l scanline) r (closest pts r scanline)

let find_endpoints pts num scanline =
  let lhs = find_endpoint pts num scanline ~left:true
  and rhs = find_endpoint pts num scanline ~left:false in
  match lhs, rhs with
    Some lhs, Some rhs -> lhs, rhs
  | _ -> raise Not_found

let get_voronoi_array pts =
  let diag = Array.create 64 [] in
  for pt = 0 to 63 do
    try
      for y = pts.(pt).y to ysize - 1 do
        let l, r = find_endpoints pts pt y in
	diag.(pt) <- (l, r, y) :: diag.(pt)
      done;
      raise Not_found
    with Not_found ->
      begin try
	for y = pts.(pt).y - 1 downto 0 do
          let l, r = find_endpoints pts pt y in
	  diag.(pt) <- (l, r, y) :: diag.(pt)
	done
      with Not_found ->
        ()
      end
  done;
  diag

let img_rgb img x y =
  match img with
    Images.Rgb24 (rgbimg) ->
      let col = Rgb24.get rgbimg x (ysize - 1 - y) in
      let { Color.Rgb.r = r; Color.Rgb.g = g; Color.Rgb.b = b } = col in
      r, g, b
  | Images.Rgba32 (rgbaimg) ->
      let { Color.Rgba.color = col } = Rgba32.get (rgbaimg) x (ysize - 1 - y) in
      let { Color.Rgb.r = r; Color.Rgb.g = g; Color.Rgb.b = b } = col in
      r, g, b
  | _ -> failwith "Unrecognized image format"

let get_region_colour img arr region =
  let racc = ref 0 and gacc = ref 0 and bacc = ref 0 and pixels = ref 0 in
  List.iter
    (fun (l, r, y) ->
      for x = l to r do
        let r, g, b = img_rgb img x y in
        racc := !racc + r;
	gacc := !gacc + g;
	bacc := !bacc + b;
	incr pixels
      done)
    arr.(region);
  !racc / !pixels, !gacc / !pixels, !bacc / !pixels

let score_region img arr region (r, g, b) =
  let diff = ref 0 and pixels = ref 0 in
  List.iter
    (fun (l, r, y) ->
      for x = l to r do
        let ir, ig, ib = img_rgb img x y in
	let dr, dg, db = ir - r, ig - g, ib - b in
	diff := !diff + (dr * dr) + (dg * dg) + (db * db);
	incr pixels
      done)
    arr.(region);
  !diff / !pixels

let colour_all_regions img pts arr =
  let score = ref 0 in
  for pt = 0 to 63 do
    let (r, g, b) as col = get_region_colour img arr pt in
    pts.(pt).c <- Graphics.rgb r g b;
    score := !score + score_region img arr pt col
  done;
  Printf.printf "score: %d\n" !score;
  flush stdout;
  !score

let render_voronoi_array pts arr =
  for pt = 0 to 63 do
    Graphics.set_color pts.(pt).c;
    List.iter
      (fun (l, r, y) ->
        Graphics.moveto l y;
	Graphics.lineto r y)
      arr.(pt)
  done

let _ =
  let img = Images.load "testimg.png" [] in
  Graphics.open_graph "";
  Graphics.set_window_title "It's a graphics window";
  Graphics.resize_window xsize ysize;
  (*Graphic_image.draw_image img 0 0;*)
  while true; do
    let pts = random_points () in
    let varr = get_voronoi_array pts in
    colour_all_regions img pts varr;
    render_voronoi_array pts varr;
  done;
  while true do
    let evs = Graphics.wait_next_event [Graphics.Button_down] in
    match evs with
      { Graphics.button = bt } when bt = true -> exit 0
    | _ -> ()
  done
