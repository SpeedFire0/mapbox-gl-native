package com.mapbox.mapboxsdk.testapp.activity.maplayout;


import android.graphics.Color;
import android.graphics.RectF;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.Toast;
import com.mapbox.android.gestures.MoveGestureDetector;
import com.mapbox.geojson.Feature;
import com.mapbox.geojson.FeatureCollection;
import com.mapbox.mapboxsdk.Mapbox;
import com.mapbox.mapboxsdk.camera.CameraUpdateFactory;
import com.mapbox.mapboxsdk.geometry.LatLng;
import com.mapbox.mapboxsdk.maps.MapView;
import com.mapbox.mapboxsdk.maps.MapboxMap;
import com.mapbox.mapboxsdk.maps.OnMapReadyCallback;
import com.mapbox.mapboxsdk.maps.Style;
import com.mapbox.mapboxsdk.style.layers.LineLayer;
import com.mapbox.mapboxsdk.style.sources.GeoJsonSource;
import com.mapbox.mapboxsdk.testapp.R;

import java.util.List;

import static com.mapbox.mapboxsdk.style.expressions.Expression.exponential;
import static com.mapbox.mapboxsdk.style.expressions.Expression.interpolate;
import static com.mapbox.mapboxsdk.style.expressions.Expression.stop;
import static com.mapbox.mapboxsdk.style.expressions.Expression.zoom;
import static com.mapbox.mapboxsdk.style.layers.PropertyFactory.lineColor;
import static com.mapbox.mapboxsdk.style.layers.PropertyFactory.lineOpacity;

/**
 * Use MapboxMap.queryRenderedFeatures() to find and highlight certain features within the map viewport.
 * The search button re-appears when the map is moved.
 */
public class SimpleMapActivity extends AppCompatActivity implements OnMapReadyCallback,
  MapboxMap.OnMoveListener {

  private static final String FILL_LAYER_ID = "FILL_LAYER_ID";
  private static final String GEO_JSON_SOURCE_ID = "GEO_JSON_SOURCE_ID";
  private static final String ID_OF_LAYER_TO_HIGHLIGHT = "road-street";
  private MapView mapView;
  private MapboxMap mapboxMap;
  private GeoJsonSource dataGeoJsonSource;
  private Button redoSearchButton;

  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);

    // This contains the MapView in XML and needs to be called after the access token is configured.
    setContentView(R.layout.activity_map_simple);

    redoSearchButton = findViewById(R.id.redo_search_button);
    mapView = findViewById(R.id.mapView);
    mapView.onCreate(savedInstanceState);
    mapView.getMapAsync(this);
  }

  @Override
  public void onMapReady(@NonNull final MapboxMap mapboxMap) {
    this.mapboxMap = mapboxMap;
    mapboxMap.moveCamera(CameraUpdateFactory.newLatLngZoom(new LatLng(34.013332, -118.312881
    ),16));
    mapboxMap.setStyle(Style.DARK, new Style.OnStyleLoaded() {
      @Override
      public void onStyleLoaded(@NonNull final Style style) {

        mapboxMap.addOnMoveListener(SimpleMapActivity.this);

        style.addSource(new GeoJsonSource(GEO_JSON_SOURCE_ID,
          FeatureCollection.fromFeatures(new Feature[] {})));

        style.addLayer(new LineLayer(FILL_LAYER_ID,
          GEO_JSON_SOURCE_ID).withProperties(
          lineOpacity(interpolate(exponential(1f), zoom(),
            stop(3, 0f),
            stop(8, .5f),
            stop(15f, 1f))),
          lineColor(Color.parseColor("#00F7FF"))
        ));

        mapboxMap.addOnCameraMoveListener(new MapboxMap.OnCameraMoveListener() {
          @Override
          public void onCameraMove() {
            Toast.makeText(SimpleMapActivity.this, "onCameraMove", Toast.LENGTH_LONG).show();
            FeatureCollection featureCollection = null;
            if (style.getLayer(ID_OF_LAYER_TO_HIGHLIGHT) != null) {
              Log.v("test", "id of layer to highlight is " + ID_OF_LAYER_TO_HIGHLIGHT);
              featureCollection = FeatureCollection.fromFeatures(getFeaturesInViewport(ID_OF_LAYER_TO_HIGHLIGHT));
            } else {
              Toast.makeText(SimpleMapActivity.this,
                "Layer not found ",
                Toast.LENGTH_SHORT).show();
            }
            // Retrieve and update the GeoJSON source used in the FillLayer
            dataGeoJsonSource = style.getSourceAs(GEO_JSON_SOURCE_ID);
            if (dataGeoJsonSource != null && featureCollection != null) {
              dataGeoJsonSource.setGeoJson(featureCollection);
            }
            redoSearchButton.setVisibility(View.INVISIBLE);
          }
        });
      }
    });
  }
//    });
//  }

  /**
   * Perform feature query within the viewport.
   */
  private List<Feature> getFeaturesInViewport(String layerName) {
    RectF rectF = new RectF(mapView.getLeft(),
      mapView.getTop(), mapView.getRight(), mapView.getBottom());
    return mapboxMap.queryRenderedFeatures(rectF, layerName);
  }

  @Override
  public void onMoveEnd(MoveGestureDetector detector) {
    redoSearchButton.setVisibility(View.VISIBLE);
  }

  @Override
  public void onMoveBegin(MoveGestureDetector detector) {
    // Left empty on purpose
  }

  @Override
  public void onMove(MoveGestureDetector detector) {
    Log.v("mapMove", "the map is moving and detector is " + detector);
  }

  @Override
  public void onResume() {
    super.onResume();
    mapView.onResume();
  }

  @Override
  protected void onStart() {
    super.onStart();
    mapView.onStart();
  }

  @Override
  protected void onStop() {
    super.onStop();
    mapView.onStop();
  }

  @Override
  public void onPause() {
    super.onPause();
    mapView.onPause();
  }

  @Override
  public void onLowMemory() {
    super.onLowMemory();
    mapView.onLowMemory();
  }

  @Override
  protected void onDestroy() {
    super.onDestroy();
    if (mapboxMap != null) {
      mapboxMap.removeOnMoveListener(this);
    }
    mapView.onDestroy();
  }

  @Override
  protected void onSaveInstanceState(Bundle outState) {
    super.onSaveInstanceState(outState);
    mapView.onSaveInstanceState(outState);
  }
}