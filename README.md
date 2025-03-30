# Транспортный справочник

Добро пожаловать в проект.

- [Описание](#описание)
- [Основной функционал](#основной-функционал)
- [Использованные идиомы и технологии](#использованные-идиомы-и-технологии)
- [Пример входных данных](#пример-входных-данных)

## Описание

Система хранения транспортных маршрутов и обработки запросов к ней:  
 &nbsp; &nbsp; • Входные данные и ответ в JSON-формате.  
 &nbsp; &nbsp; • Выходной JSON-файл может содержать визуализацию карты маршрута(ов) в формате SVG-файла.  
 &nbsp; &nbsp; • Построение оптимальных путей (поиск маршрутов и вычисление времени поездки).  

## Основной функционал

Хранение данных	-	unordered_map, кастомное хеширование  
Обработка JSON	-	самописный парсер, std::variant  
Визуализация	-	собственная SVG библиотека  
Поиск пути	-	теория графов, взвешенные ребра  

## Использованные идиомы и технологии
&nbsp; &nbsp; • Модульная архитектура (разделение на парсер, ядро, визуализацию)  
&nbsp; &nbsp; • ООП  
&nbsp; &nbsp; • Unordered map/set  
&nbsp; &nbsp; • STL smart pointers  
&nbsp; &nbsp; • std::variant and std:optional  
&nbsp; &nbsp; • JSON load / output  
&nbsp; &nbsp; • SVG image format    

## Пример входных данных

`base_requests` — описание автобусных маршрутов и остановок.  
`stat_requests` — запросы к транспортному справочнику.  
`render_settings` — настройки рендеринга карты в формате SVG.  
`routing_settings` — настройки роутера для поиска кратчайших маршрутов.  

```json
  {
      "base_requests": [
          {
              "is_roundtrip": true,
              "name": "297",
              "stops": [
                  "Biryulyovo Zapadnoye",
                  "Biryulyovo Tovarnaya",
                  "Universam",
                  "Biryulyovo Zapadnoye"
              ],
              "type": "Bus"
          },
          {
              "is_roundtrip": false,
              "name": "635",
              "stops": [
                  "Biryulyovo Tovarnaya",
                  "Universam",
                  "Prazhskaya"
              ],
              "type": "Bus"
          },
          {
              "latitude": 55.574371,
              "longitude": 37.6517,
              "name": "Biryulyovo Zapadnoye",
              "road_distances": {
                  "Biryulyovo Tovarnaya": 2600
              },
              "type": "Stop"
          },
          {
              "latitude": 55.587655,
              "longitude": 37.645687,
              "name": "Universam",
              "road_distances": {
                  "Biryulyovo Tovarnaya": 1380,
                  "Biryulyovo Zapadnoye": 2500,
                  "Prazhskaya": 4650
              },
              "type": "Stop"
          },
          {
              "latitude": 55.592028,
              "longitude": 37.653656,
              "name": "Biryulyovo Tovarnaya",
              "road_distances": {
                  "Universam": 890
              },
              "type": "Stop"
          },
          {
              "latitude": 55.611717,
              "longitude": 37.603938,
              "name": "Prazhskaya",
              "road_distances": {},
              "type": "Stop"
          }
      ],
      "render_settings": {
          "bus_label_font_size": 20,
          "bus_label_offset": [
              7,
              15
          ],
          "color_palette": [
              "green",
              [
                  255,
                  160,
                  0
              ],
              "red"
          ],
          "height": 200,
          "line_width": 14,
          "padding": 30,
          "stop_label_font_size": 20,
          "stop_label_offset": [
              7,
              -3
          ],
          "stop_radius": 5,
          "underlayer_color": [
              255,
              255,
              255,
              0.85
          ],
          "underlayer_width": 3,
          "width": 200
      },
      "routing_settings": {
          "bus_velocity": 40,
          "bus_wait_time": 6
      },
      "stat_requests": [
          {
              "id": 1, 
              "type": "Map"
          },
          {
              "id": 2,
              "name": "635",
              "type": "Bus"
          },
          {
              "id": 3,
              "name": "Universam",
              "type": "Stop"
          },
          {
              "from": "Biryulyovo Zapadnoye",
              "id": 4,
              "to": "Universam",
              "type": "Route"
          },
          {
              "from": "Biryulyovo Zapadnoye",
              "id": 5,
              "to": "Prazhskaya",
              "type": "Route"
          }
      ]
  }
```

## Полученный JSON документ

``` json
[
    {
        "map": "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n<polyline points=\"164.492,135.162 170,85.4419 147.56,97.7557 164.492,135.162\" fill=\"none\" stroke=\"green\" stroke-width=\"14\" stroke-linecap=\"round\" stroke-linejoin=\"round\" />\n<polyline points=\"170,85.4419 147.56,97.7557 30,30 147.56,97.7557 170,85.4419\" fill=\"none\" stroke=\"rgb(255,160,0)\" stroke-width=\"14\" stroke-linecap=\"round\" stroke-linejoin=\"round\" />\n<text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"164.492\" y=\"135.162\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\">297</text>\n<text fill=\"green\" x=\"164.492\" y=\"135.162\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\">297</text>\n<text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"170\" y=\"85.4419\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\">635</text>\n<text fill=\"rgb(255,160,0)\" x=\"170\" y=\"85.4419\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\">635</text>\n<text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"30\" y=\"30\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\">635</text>\n<text fill=\"rgb(255,160,0)\" x=\"30\" y=\"30\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\">635</text>\n<circle cx=\"170\" cy=\"85.4419\" r=\"5\" fill=\"white\"/>\n<circle cx=\"164.492\" cy=\"135.162\" r=\"5\" fill=\"white\"/>\n<circle cx=\"30\" cy=\"30\" r=\"5\" fill=\"white\"/>\n<circle cx=\"147.56\" cy=\"97.7557\" r=\"5\" fill=\"white\"/>\n<text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"170\" y=\"85.4419\" dx=\"7\" dy=\"-3\" font-size=\"20\" font-family=\"Verdana\">Biryulyovo Tovarnaya</text>\n<text fill=\"black\" x=\"170\" y=\"85.4419\" dx=\"7\" dy=\"-3\" font-size=\"20\" font-family=\"Verdana\">Biryulyovo Tovarnaya</text>\n<text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"164.492\" y=\"135.162\" dx=\"7\" dy=\"-3\" font-size=\"20\" font-family=\"Verdana\">Biryulyovo Zapadnoye</text>\n<text fill=\"black\" x=\"164.492\" y=\"135.162\" dx=\"7\" dy=\"-3\" font-size=\"20\" font-family=\"Verdana\">Biryulyovo Zapadnoye</text>\n<text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"30\" y=\"30\" dx=\"7\" dy=\"-3\" font-size=\"20\" font-family=\"Verdana\">Prazhskaya</text>\n<text fill=\"black\" x=\"30\" y=\"30\" dx=\"7\" dy=\"-3\" font-size=\"20\" font-family=\"Verdana\">Prazhskaya</text>\n<text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"147.56\" y=\"97.7557\" dx=\"7\" dy=\"-3\" font-size=\"20\" font-family=\"Verdana\">Universam</text>\n<text fill=\"black\" x=\"147.56\" y=\"97.7557\" dx=\"7\" dy=\"-3\" font-size=\"20\" font-family=\"Verdana\">Universam</text>\n</svg>\n",
        "request_id": 1
    },
    {
        "curvature": 1.30156,
        "request_id": 2,
        "route_length": 11570,
        "stop_count": 5,
        "unique_stop_count": 3
    },
    {
        "buses": [
            "297",
            "635"
        ],
        "request_id": 3
    },
    {
        "items": [
            {
                "stop_name": "Biryulyovo Zapadnoye",
                "time": 6,
                "type": "Wait"
            },
            {
                "bus": "297",
                "span_count": 2,
                "time": 5.235,
                "type": "Bus"
            }
        ],
        "request_id": 4,
        "total_time": 11.235
    },
    {
        "items": [
            {
                "stop_name": "Biryulyovo Zapadnoye",
                "time": 6,
                "type": "Wait"
            },
            {
                "bus": "297",
                "span_count": 1,
                "time": 3.9,
                "type": "Bus"
            },
            {
                "stop_name": "Biryulyovo Tovarnaya",
                "time": 6,
                "type": "Wait"
            },
            {
                "bus": "635",
                "span_count": 2,
                "time": 8.31,
                "type": "Bus"
            }
        ],
        "request_id": 5,
        "total_time": 24.21
    }
]
```
