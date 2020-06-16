/*
 * Copyright (C) 2020 Bence Sipka
 *
 * This program is free software: you can redistribute it and/or modify 
 * it under the terms of the GNU General Public License as published by 
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <framework/animation/Animation.h>
#include <framework/animation/PropertyAnimator.h>
#include <framework/core/timing.h>
#include <framework/core/Window.h>
#include <framework/geometry/Matrix.h>
#include <framework/io/key/KeyEvent.h>
#include <framework/io/touch/TouchEvent.h>
#include <framework/layer/Layer.h>
#include <framework/render/Renderer.h>
#include <framework/utils/LifeCycleChain.h>
#include <framework/io/stream/LoopedInputStream.h>
#include <framework/io/stream/OutputStream.h>
#include <framework/io/stream/InputStream.h>
#include <framework/io/touch/gesture/scroll/ScrollGestureDetector.h>
#include <gen/types.h>
#include <gen/log.h>
#include <sapphire/dialogs/LevelDetailsLayer.h>
#include <sapphire/community/CommunityLayer.h>
#include <sapphire/SapphireScene.h>
#include <sapphire/dialogs/DialogLayer.h>
#include <sapphire/dialogs/ParentDismisserDialogLayer.h>
#include <sapphire/AsynchronTask.h>
#include <sapphire/server/SapphireLevelDetails.h>
#include <sapphire/dialogs/items/LevelRatingDialogItem.h>
#include <sapphire/FrameAnimation.h>
#include <sapphire/dialogs/items/EditTextDialogItem.h>
#include <sapphire/dialogs/items/BusyIndicatorDialogItem.h>

#define _USE_MATH_DEFINES
#include <math.h>

using namespace rhfw;
#define SEND_MESSAGE_TEXT "Send"

namespace userapp {

class DetailsPointer {
public:
	const SapphireLevelDetails* details;
	const SapphireLevelDescriptor* descriptor;
	LevelState state = LevelState::UNSEEN;
};

#define TITLE_TEXT "Community"

#define BREAK_ON_NOT_DOWN()  if (KeyEvent::instance.getAction() != KeyAction::DOWN) break

class LevelsCommunityPage: public CommunityLayer::CommunityPage, private CommunityConnection::StateListener {
private:
	typedef int (*LevelComparatorFunction)(const SapphireLevelDetails& left, const SapphireLevelDetails& right);
	static const int COLUMN_COUNT = 4;
	class DetailsColumn {
	public:
		const char* title;
		float weight;
		float length;
		float textAlign;

		Gravity textGravity;

		Rectangle rect;
		bool pointerDown = false;

		LevelComparatorFunction columnComparator;
	};
	static int SameComparator(const SapphireLevelDetails& left, const SapphireLevelDetails& right) {
		return left.uuid.compare(right.uuid);
	}
	static int DefaultLevelComparator(const SapphireLevelDetails& left, const SapphireLevelDetails& right) {
		if (left.difficulty == right.difficulty) {
			return left.title.compare(right.title);
		}
		return (int) left.difficulty - (int) right.difficulty;
	}
	template<int (*FirstComparator)(const SapphireLevelDetails& left, const SapphireLevelDetails& right), int (*SecondComparator)(
			const SapphireLevelDetails& left, const SapphireLevelDetails& right)>
	static int EqualityComparator(const SapphireLevelDetails& left, const SapphireLevelDetails& right) {
		int res = FirstComparator(left, right);
		if (res != 0) {
			return res;
		}
		return SecondComparator(left, right);
	}

	class DownloadLevelDetailsLayer: public DialogLayer {
	private:
		CommunityConnection::LevelDownloadListener::Listener downloadListener;
	public:
		DownloadLevelDetailsLayer(LevelsCommunityPage* page, const DetailsPointer& details)
				: DialogLayer { page->parent } {
			CommunityLayer* parent = page->parent;
			setColors(details.details->difficulty);

			setTitle(details.details->title);
			addDialogItem(new BusyIndicatorDialogItem("Downloading"));
			addDialogItem(new EmptyDialogItem(0.5f));
			addDialogItem(new CommandDialogItem("Cancel", [=] {
				downloadListener = nullptr;
				this->dismiss();
			}));
			downloadListener =
					CommunityConnection::LevelDownloadListener::make_listener(
							[=](const SapphireUUID& uuid, SapphireCommError error, const Level* level) {
								if (uuid == details.details->uuid ) {
									switch (error) {
										case SapphireCommError::NoError: {
											auto* descriptor = static_cast<SapphireScene*>(parent->getScene())->getDescriptorForDownloadedLevel(*level);
											page->setLevelDescriptor(details.details, descriptor);

											this->dismiss();
											LevelDetailsLayer* dialog = new LevelDetailsLayer(parent, descriptor, *level);
											dialog->show(getScene(), true);
											break;
										}
										case SapphireCommError::NewerVersion: {
											this->dismiss();
											DialogLayer* info = new DialogLayer(parent);
											info->setTitle("Upgrade required");
											info->addDialogItem(new TextDialogItem("To play this level, please upgrade " SAPPHIRE_GAME_NAME " to a newer version."));
											info->addDialogItem(new EmptyDialogItem(0.5f));
											info->addDialogItem(new CommandDialogItem( "Okay", [=] {
																info->dismiss();
															}));
											info->show(getScene(), true);
											break;
										}
										default: {
											this->dismiss();
											DialogLayer* info = new DialogLayer(parent);
											info->setTitle("Download failed");
											info->addDialogItem(new TextDialogItem("Failed to download level. Internal server error."));
											info->addDialogItem(new EmptyDialogItem(0.5f));
											info->addDialogItem(new CommandDialogItem( "Back", [=] {
																info->dismiss();
															}));
											info->show(getScene(), true);
											break;
										}
									}
								}
							});
			//TODO with state listener
			static_cast<SapphireScene*>(parent->getScene())->getConnection().levelDownloadEvents += downloadListener;
			static_cast<SapphireScene*>(parent->getScene())->getConnection().downloadLevel(details.details);
		}
	};

	AutoResource<Font> font = getFont(ResIds::build::sipka_rh_font_convert::gameres::Consolas_ttf);
	AutoResource<render::Texture> starFilled = getTexture(ResIds::gameres::game_sapphire::art::ic_star_filled);
	AutoResource<FrameAnimation> markers = getAnimation(ResIds::gameres::game_sapphire::level_markers);

	ScrollGestureDetector scrollDetector;
	float scroll = 0.0f;

	DetailsColumn columns[COLUMN_COUNT];

	float rowHeight = 0.0f;

	unsigned int levelsCapacity = 256;
	unsigned int levelsCount = 0;

	DetailsPointer* levels = new DetailsPointer[levelsCapacity];

	LevelComparatorFunction levelComparatorFunction = DefaultLevelComparator;
	/**
	 * 1 for ascending, -1 for descending
	 */
	int sortingDirection = 1;

	int selected = -1;
	int lastSelected = -1;
	int touchRow = -1;

	CommunityConnection::LevelsQueriedListener::Listener queryListener = CommunityConnection::LevelsQueriedListener::make_listener(
			[=](unsigned int index, const SapphireLevelDetails* details) {
				if(details != nullptr) {
					if(index >= levelsCount) {
						addLevel(details);
						loadMoreVisible = LOADMORE_VISIBLE;
						updateScrollDetectorSize();
					}
				} else {
					if(index == scene->getConnection().getLevelDetails().size()) {
						loadMoreVisible = LOADMORE_INVISIBLE;
						updateScrollDetectorSize();
						if(touchRow < 0) {
							/*no touch in progress, readjust scroll*/
							animateIndex(lastSelected);
							if(selected == levelsCount && levelsCount > 0) {
								selected = levelsCount - 1;
							}
						}
					}
				}
			});
	SapphireScene::UnknownLevelStateListener::Listener unknownLevelStateListener = SapphireScene::UnknownLevelStateListener::make_listener(
			[=](const SapphireUUID& uuid, LevelState state) {
				for (unsigned int i = 0; i < levelsCount; ++i) {
					if (levels[i].descriptor == nullptr && levels[i].details->uuid == uuid) {
						levels[i].state = state;
						break;
					}
				}
			});
	CommunityConnection::LevelRemovedListener::Listener levelRemovedListener = CommunityConnection::LevelRemovedListener::make_listener(
			[=](const SapphireLevelDetails* details) {
				int index = getIndexForDetails(details, levelComparatorFunction, sortingDirection);
				if(index >= 0) {
					memmove(levels + index, levels + index + 1, sizeof(DetailsPointer) * (levelsCount - index - 1));
					if(selected > index) {
						--selected;
					}
					if(lastSelected > index) {
						--lastSelected;
					}
					--levelsCount;
					updateScrollDetectorSize();
				}
			});

	template<typename Comparator>
	int getIndexForDetails(const SapphireLevelDetails* details, Comparator&& comp, int sortingdir) {
		int low = 0;
		int mid = levelsCount;
		int high = mid - 1;
		int cmpresult = 1;
		while (low <= high) {
			mid = low + (high - low) / 2;
			cmpresult = comp(*levels[mid].details, *details) * sortingdir;
			if (cmpresult == 0) {
				cmpresult = SameComparator(*levels[mid].details, *details);
			}
			if (cmpresult == 0) {
				return mid;
			} else if (cmpresult < 0) {
				low = mid + 1;
			} else {
				high = mid - 1;
			}
		}
		return -mid - (cmpresult > 0 ? 1 : 2);
	}

	void addLevel(const SapphireLevelDetails* details) {
#ifndef SAPPHIRE_DUAL_PLAYER_AVAILABLE
		if(details->playerCount > 1) {
			return;
		}
#endif /*SAPPHIRE_DUAL_PLAYER_AVAILABLE*/
		if (levelsCount + 1 > levelsCapacity) {
			auto* old = levels;
			levelsCapacity *= 2;
			levels = new DetailsPointer[levelsCapacity];
			memcpy(levels, old, sizeof(DetailsPointer) * levelsCount);
			delete[] old;
		}
		int insertindex = getIndexForDetails(details, levelComparatorFunction, sortingDirection);
		if (insertindex < 0) {
			//can be >= 0, when the same sort order found, but different levels
			insertindex = -(insertindex + 1);
		}
		ASSERT(insertindex <= levelsCount);
		if (insertindex < levelsCount) {
			memmove(levels + insertindex + 1, levels + insertindex, sizeof(DetailsPointer) * (levelsCount - insertindex));
		}
		auto& ptr = levels[insertindex];
		ptr.details = details;
		ptr.descriptor = scene->getLevelWithUUID(details->uuid);
		if (ptr.descriptor == nullptr) {
			ptr.state = scene->getUnknownLevelState(details->uuid);
		}
		if (lastSelected < 0) {
			lastSelected = 0;
		}
		++levelsCount;
		updateScrollDetectorSize();
	}

	void setLevelDescriptor(const SapphireLevelDetails* details, const SapphireLevelDescriptor* descriptor) {
		int index = getIndexForDetails(details, levelComparatorFunction, sortingDirection);
		ASSERT(index >= 0);
		if (index < 0) {
			//just in case
			return;
		}
		levels[index].descriptor = descriptor;
	}

	void executeSort() {
		for (int i = 0; i < (int) levelsCount - 1; ++i) {
			for (int j = i + 1; j < (int) levelsCount; ++j) {
				int cmp = levelComparatorFunction(*levels[i].details, *levels[j].details) * sortingDirection;
				if (cmp == 0) {
					cmp = SameComparator(*levels[i].details, *levels[j].details);
				}
				if (cmp > 0) {
					//swap
					auto tmp = util::move(levels[i]);
					levels[i] = util::move(levels[j]);
					levels[j] = util::move(tmp);
				}
			}
		}
	}

	void sort(LevelComparatorFunction comparator, int sortingdirection) {
		this->levelComparatorFunction = comparator;
		this->sortingDirection = sortingdirection;

		executeSort();
	}
	void sort(LevelComparatorFunction comparator) {
		if (levelComparatorFunction == comparator) {
			if (sortingDirection < 0) {
				//sorting for this column, descending, remove sorting
				levelComparatorFunction = DefaultLevelComparator;
				sortingDirection = 1;
			} else {
				//sort descending
				sortingDirection = -1;
			}
		} else {
			sortingDirection = 1;
			levelComparatorFunction = comparator;
		}
		//execute sorting

		executeSort();
	}

	Rectangle rect;

	static const int LOADMORE_INVISIBLE = 0;
	static const int LOADMORE_VISIBLE = 1;
	static const int LOADMORE_LOADING = 2;

	int loadMoreVisible = LOADMORE_LOADING;

	LifeCycleChain<Animation> scrollAnimationHolder;

#define nameColumn (columns[0])
#define authorColumn (columns[1])
#define dateColumn (columns[2])
#define ratingsColumn (columns[3])

public:
	LevelsCommunityPage(SapphireScene* scene, CommunityLayer* parent)
			: CommunityPage(scene, parent) {
		nameColumn.title = "Name";
		authorColumn.title = "Author";
		dateColumn.title = "Date";
		ratingsColumn.title = "Rating";

		nameColumn.textGravity = Gravity::CENTER_VERTICAL | Gravity::LEFT;
		authorColumn.textGravity = Gravity::CENTER;
		dateColumn.textGravity = Gravity::CENTER;
		ratingsColumn.textGravity = Gravity::CENTER;

		nameColumn.weight = 0.0f;
		authorColumn.weight = 0.4f;
		dateColumn.weight = 0.6f;
		ratingsColumn.weight = 0.8f;

		nameColumn.length = 0.4f;
		authorColumn.length = 0.2f;
		dateColumn.length = 0.2f;
		ratingsColumn.length = 0.2f;

		nameColumn.textAlign = 0.0f;
		authorColumn.textAlign = authorColumn.length / 2.0f;
		dateColumn.textAlign = dateColumn.length / 2.0f;
		ratingsColumn.textAlign = ratingsColumn.length / 2.0f;

		nameColumn.columnComparator = EqualityComparator<SapphireLevelDetails::compareNames, DefaultLevelComparator>;
		authorColumn.columnComparator = EqualityComparator<SapphireLevelDetails::compareAuthors, DefaultLevelComparator>;
		dateColumn.columnComparator = EqualityComparator<SapphireLevelDetails::compareDates, DefaultLevelComparator>;
		ratingsColumn.columnComparator = EqualityComparator<SapphireLevelDetails::compareRatings, DefaultLevelComparator>;

		auto&& conn = scene->getConnection();
		conn.levelsQueriedEvents += queryListener;
		conn.levelRemovedEvents += levelRemovedListener;
		scene->unknownLevelStateListeners += unknownLevelStateListener;
		for (auto* details : conn.getLevelDetails()) {
			if (details != nullptr) {
				addLevel(details);
			}
		}
	}
	~LevelsCommunityPage() {
		delete[] levels;
	}

	virtual void onCommunityConnected() override {
		scene->getConnection().requestLevels();
	}
	virtual void onCommunityDisconnected() override {
		levelsCount = 0;

		selected = -1;
		lastSelected = -1;
		touchRow = -1;

		updateScrollDetectorSize();
	}

	virtual void draw(const Matrix2D& amvp, float alpha) override {
		renderer->initDraw();

		FastFontDrawer& fontdrawer = parent->getFontDrawer();

		float scroll = -scrollDetector.getPosition().y();

		Matrix2D mvp;
		mvp.setTranslate(0, -scroll);
		mvp *= amvp;
		const float rowtextsize = rowHeight * 0.9f;
		{
			for (int i = 0; i < COLUMN_COUNT; ++i) {
				Vector2F pos { rect.left + (columns[i].weight + columns[i].length / 2.0f) * (rect.width()), rect.top + rowHeight / 2.0f };
				if (columns[i].pointerDown) {
					drawRectangleColor(amvp, Color { parent->getUiColor().rgb(), alpha }, columns[i].rect);
				}
				float len = fontdrawer.add(columns[i].title, Color { parent->getSelectedUiColor(columns[i].pointerDown).rgb(), alpha }, pos,
						rowtextsize, Gravity::CENTER);
				if (levelComparatorFunction == columns[i].columnComparator) {
					fontdrawer.add(sortingDirection < 0 ? "(D)" : "(A)", Color { parent->getSelectedUiColor(columns[i].pointerDown).rgb(),
							alpha }, Vector2F { pos.x() + len / 2.0f, pos.y() }, rowtextsize, Gravity::LEFT | Gravity::CENTER_VERTICAL);
				}
			}
		}
		unsigned int normallast = (unsigned int) ((scroll + rect.height()) / rowHeight);
		unsigned int first = (unsigned int) (scroll / rowHeight);
		unsigned int last = min(normallast, levelsCount - 1);
		//TODO ellipsize long texts
		if (levelsCount > 0) {
			for (unsigned int i = first; i <= last; ++i) {
				Vector2F itempos { rect.left, rect.top + (i + 1.0f) * rowHeight };
				ASSERT(levels[i].details != nullptr) << i;
				auto& detail = *levels[i].details;
				auto desc = levels[i].descriptor;
				LevelState levelstate = desc == nullptr ? levels[i].state : desc->state;
				float rowalpha = i == first ? 1.0f - (scroll - first * rowHeight) / rowHeight : 1.0f;
				rowalpha *= rowalpha;
				//rowalpha = min(1.0f, (scroll - i * rowHeight) / rowHeight);
				Vector2F pos { rect.left, rect.top + (i + 1) * rowHeight };
				float texty = rect.top + (i + 1.5f) * rowHeight - scroll;
				Color color = difficultyToColor(detail.difficulty);
				Color selectedcolor = difficultyToSelectedColor(detail.difficulty);
				const Color& textcolor = Color { (selected == i ? selectedcolor : color).xyz(), alpha * rowalpha };

				float markerwidth = rowHeight * 2.0f / 3.0f;
				float markerspadding = rowHeight * 0.05f;
				float itemleftpadding = markerwidth * 3;

				if (selected == i) {
					drawRectangleColor(mvp, Color { color.xyz(), alpha * rowalpha },
							Rectangle { pos, Vector2F { pos.x() + rect.width(), pos.y() + rowHeight } });
				}
				if (levelstate == LevelState::UNSEEN || levelstate == LevelState::COMPLETED) {
					auto& elem = markers->getAtIndex(
							(unsigned int) (
									levelstate == LevelState::COMPLETED ?
											SapphireMarkerAtlas::MARKER_TICK : SapphireMarkerAtlas::MARKER_QUESTION));
					drawSapphireTexture(mvp, elem, textcolor,
							Rectangle { itempos, Vector2F { itempos.x() + markerwidth, itempos.y() + rowHeight } }.inset(markerspadding).fitInto(
									Rectangle { 0, 0, 14, 18 }), elem.getPosition());
				}
				if (desc != nullptr && desc->hasSuspendedGame) {
					auto& elem = markers->getAtIndex((unsigned int) SapphireMarkerAtlas::MARKER_SUSPEND);
					drawSapphireTexture(mvp, elem, textcolor,
							Rectangle { itempos.x() + markerwidth, itempos.y(), itempos.x() + markerwidth * 2, itempos.y() + rowHeight }.fitInto(
									Rectangle { 0, 0, 17, 23 }).inset(markerspadding), elem.getPosition());
				} else {
					drawRectangleColor(mvp, textcolor,
							Rectangle { 0, 0, rowHeight / 5.0f, rowHeight / 5.0f }.centerInto(
									Rectangle { itempos.x() + markerwidth + markerspadding, itempos.y(), itempos.x() + markerwidth * 2
											+ markerspadding, itempos.y() + rowHeight }));
				}
				if (detail.category != SapphireLevelCategory::None) {
					float xpos = itempos.x() + markerwidth * 2;
					auto& elem = markers->getAtIndex((unsigned int) SapphireMarkerAtlas::CATEGORY_START + (unsigned int) detail.category);
					drawSapphireTexture(mvp, elem, textcolor,
							Rectangle { xpos, itempos.y(), xpos + markerwidth, itempos.y() + rowHeight }.fitInto(Rectangle { 0, 0, 17, 23 }).inset(
									markerspadding), elem.getPosition());
				}
				float nametextleft = rect.left + (nameColumn.weight + nameColumn.textAlign) * rect.width() + itemleftpadding;
				if (detail.playerCount > 1) {
					nametextleft += fontdrawer.add("(2) ", textcolor, Vector2F { nametextleft, texty }, rowtextsize,
					nameColumn.textGravity);
				}
				{
					//ellipsizing text

					auto* end = fontdrawer.getFont()->measureText((const char*) detail.title, rowtextsize,
					nameColumn.length * rect.width() - nametextleft + rect.left);
					if (*end == 0) {
						//whole title fit
						fontdrawer.add(detail.title, textcolor, Vector2F { nametextleft, texty }, rowtextsize, nameColumn.textGravity);
					} else {
						unsigned int len = end - (const char*) detail.title;
						if (len <= 4) {
							fontdrawer.add(detail.title, end, textcolor, Vector2F { nametextleft, texty }, rowtextsize,
							nameColumn.textGravity);
						} else {
							float titlen = fontdrawer.add(detail.title, end - 2, textcolor, Vector2F { nametextleft, texty }, rowtextsize,
							nameColumn.textGravity);
							fontdrawer.add("..", textcolor, Vector2F { nametextleft + titlen, texty }, rowtextsize, nameColumn.textGravity);
						}
					}
				}

				char buffer[64];

				fontdrawer.add(detail.author, textcolor,
						Vector2F { rect.left + (authorColumn.weight + authorColumn.textAlign) * rect.width(), texty }, rowtextsize,
						authorColumn.textGravity);

				sprintf(buffer, "%04u.%02u.%02u", detail.dateYear, detail.dateMonth, detail.dateDay);

				fontdrawer.add(buffer, textcolor, Vector2F { rect.left + (dateColumn.weight + dateColumn.textAlign) * rect.width(), texty },
						rowtextsize,
						dateColumn.textGravity);

				const char* ratingtext;
				if (detail.ratingCount == 0) {
					ratingtext = "-";
				} else {
					sprintf(buffer, "%.1f(%u)", (float) detail.ratingSum / detail.ratingCount, detail.ratingCount);
					ratingtext = buffer;
				}
				Vector2F ratingpos { rect.left + rowHeight + (ratingsColumn.weight + ratingsColumn.textAlign) * rect.width(), texty };
				float ratinglen = fontdrawer.add(ratingtext, textcolor, ratingpos, rowtextsize, ratingsColumn.textGravity);
				if (ratingtext == buffer) {
					//it is assumed, that ratingscolumn gravity is center
					//draw star
					drawSapphireTexture(mvp, starFilled, textcolor,
							Rectangle { ratingpos.x() - ratinglen / 2.0f - rowHeight * 1.15f, rect.top + (i + 1.0f) * rowHeight,
									ratingpos.x() - ratinglen / 2.0f - rowHeight * 0.15f, rect.top + (i + 2.0f) * rowHeight }, Rectangle {
									0, 0, 1, 1 });
				}
			}
		} else {
			fontdrawer.add("No levels available", Color { parent->getUiColor().rgb(), alpha },
					Vector2F { rect.middle().x(), rect.top + rect.height() * 0.25f }, rowtextsize * 1.5, Gravity::CENTER);
		}
		if (normallast >= levelsCount && loadMoreVisible != LOADMORE_INVISIBLE) {
			Vector2F pos { rect.left, rect.top + (last + 2) * rowHeight };
			Vector2F textpos { rect.left + rect.width() / 2.0f, rect.top + (last + 2.5f) * rowHeight - scroll };
			const char* text = loadMoreVisible == LOADMORE_LOADING ? "Loading levels..." : "Load more...";
			const Color& textcol = selected == levelsCount ? parent->getUiSelectedColor() : parent->getUiColor();

			if (selected == levelsCount) {
				drawRectangleColor(mvp, Color { parent->getUiColor().rgb(), alpha },
						Rectangle { pos, Vector2F { pos.x() + rect.width(), pos.y() + rowHeight } });
			}
			fontdrawer.add(text, Color { textcol.rgb(), alpha }, textpos, rowtextsize, Gravity::CENTER);
		}
	}
	virtual bool onKeyEvent() override {
		switch (KeyEvent::instance.getKeycode()) {
			case KeyCode::KEY_GAMEPAD_DPAD_UP:
			case KeyCode::KEY_DIR_UP: {
				BREAK_ON_NOT_DOWN();
				if(selected < 0) {
					setSelectedIndex(lastSelected);
				} else if(selected > 0) {
					setSelectedIndex(selected - 1);
				}
				break;
			}
			case KeyCode::KEY_GAMEPAD_DPAD_DOWN:
			case KeyCode::KEY_DIR_DOWN: {
				BREAK_ON_NOT_DOWN();
				if(selected < 0) {
					setSelectedIndex(lastSelected);
				} else {
					setSelectedIndex(selected + 1);
				}
				break;
			}
			case KeyCode::KEY_GAMEPAD_A:
			case KeyCode::KEY_ENTER: {
				BREAK_ON_NOT_DOWN();
				if(selected < 0) {
					setSelectedIndex(lastSelected);
				} else {
					if(selected == levelsCount) {
						if(loadMoreVisible == LOADMORE_VISIBLE) {
							loadMoreLevels();
						}
					} else {
						selectLevel(selected);
					}
				}
				break;
			}
			case KeyCode::KEY_HOME: {
				BREAK_ON_NOT_DOWN();
				if (levelsCount > 0) {
					setSelectedIndex(0);
				}
				break;
			}
			case KeyCode::KEY_END: {
				BREAK_ON_NOT_DOWN();
				if (levelsCount > 0) {
					if(loadMoreVisible != LOADMORE_INVISIBLE) {
						setSelectedIndex(levelsCount);
					} else {
						setSelectedIndex(levelsCount - 1);
					}
				}
				break;
			}
			case KeyCode::KEY_GAMEPAD_RIGHT_TRIGGER:
			case KeyCode::KEY_PAGE_DOWN: {
				if(levelsCount == 0) {
					return false;
				}
				BREAK_ON_NOT_DOWN();
				if (levelsCount > 0) {
					if(selected < 0) {
						setSelectedIndex(lastSelected);
					} else {
						setSelectedIndex(selected + (unsigned int)(rect.height() / rowHeight));
					}
				}
				break;
			}
			case KeyCode::KEY_GAMEPAD_LEFT_TRIGGER:
			case KeyCode::KEY_PAGE_UP: {
				if(levelsCount == 0) {
					return false;
				}
				BREAK_ON_NOT_DOWN();
				if (levelsCount > 0) {
					if(selected < 0) {
						setSelectedIndex(lastSelected);
					} else {
						unsigned int index = selected - (unsigned int)(rect.height() / rowHeight);
						if(index >= levelsCount) {
							//in case of overflow
							index = 0;
						}
						setSelectedIndex(index);
					}
				}
				break;
			}
			case KeyCode::KEY_1: {
				BREAK_ON_NOT_DOWN();
				sort(columns[0].columnComparator);
				break;
			}
			case KeyCode::KEY_2: {
				BREAK_ON_NOT_DOWN();
				sort(columns[1].columnComparator);
				break;
			}
			case KeyCode::KEY_3: {
				BREAK_ON_NOT_DOWN();
				sort(columns[2].columnComparator);
				break;
			}
			case KeyCode::KEY_4: {
				BREAK_ON_NOT_DOWN();
				sort(columns[3].columnComparator);
				break;
			}
			default: {
				return false;
			}
		}
		return true;
	}
	bool isAnyColumnPointerDown() {
		for (unsigned int i = 0; i < COLUMN_COUNT; ++i) {
			if (columns[i].pointerDown) {
				return true;
			}
		}
		return false;
	}
	virtual void touch(TouchAction action, const Vector2F& touchpos) override {
		scrollAnimationHolder.kill();
		if (action == TouchAction::CANCEL) {
			selected = -1;
			touchRow = -1;
			for (unsigned int i = 0; i < COLUMN_COUNT; ++i) {
				columns[i].pointerDown = false;
			}
			return;
		}
		float scroll = -scrollDetector.getPosition().y();
		scrollDetector.onTouch(touchRow < 0 && !isAnyColumnPointerDown());
		switch (action) {
			case TouchAction::DOWN: {
				selected = -1;
				for (unsigned int i = 0; i < COLUMN_COUNT; ++i) {
					if(columns[i].rect.isInside(touchpos)) {
						columns[i].pointerDown = true;
						return;
					}
				}
				if (rect.isInside(touchpos) && touchpos.y() > rowHeight) {
					unsigned int index = (unsigned int)((scroll + touchpos.y() - rect.top) / rowHeight) - 1;
					if(index < levelsCount || (loadMoreVisible != LOADMORE_INVISIBLE && index == levelsCount)) {
						touchRow = index;
						lastSelected = selected = index;
					}
				}
				break;
			}
			case TouchAction::MOVE_UPDATE: {
				for (unsigned int i = 0; i < COLUMN_COUNT; ++i) {
					if (columns[i].pointerDown) {
						if (!columns[i].rect.isInside(touchpos)) {
							columns[i].pointerDown = false;
						}
						return;
					}
				}
				if(touchRow >= 0
						&& (!rect.isInside(touchpos) || touchpos.y() <= rowHeight || (unsigned int)((scroll + touchpos.y() - rect.top) / rowHeight) - 1 != touchRow)) {
					touchRow = -1;
					selected = -1;
				}

				break;
			}
			case TouchAction::UP: {
				for (unsigned int i = 0; i < COLUMN_COUNT; ++i) {
					if (columns[i].pointerDown) {
						columns[i].pointerDown = false;
						sort(columns[i].columnComparator);
						return;
					}
				}
				if(touchRow >= 0
						&& !(!rect.isInside(touchpos) || touchpos.y() <= rowHeight || (unsigned int)((scroll + touchpos.y() - rect.top) / rowHeight) - 1 != touchRow)) {
					if(touchRow == levelsCount && loadMoreVisible == LOADMORE_VISIBLE) {
						loadMoreLevels();
					} else {
						selectLevel(touchRow);
					}
					setSelectedIndex(touchRow);
				}
				selected = -1;
				touchRow = -1;
				break;
			}
			default: {
				break;
			}
		}
	}
	void updateScrollDetectorSize() {
		scrollDetector.setSize(
				Vector2F {rect.width(), rect.height() - rowHeight},
				Vector2F {rect.width(), (levelsCount + (loadMoreVisible == LOADMORE_INVISIBLE ? 0 : 1)) * rowHeight}
		);
	}
	virtual void sizeChanged(const rhfw::core::WindowSize& size) override {
		float padding = min(size.toPixelsX(0.5f), size.pixelSize.width() / 12.0f);
		this->rect = {padding, padding / 3.0f, size.pixelSize.width() - padding, (float) size.pixelSize.height()};
		const int MIN_ROWS_A_COLUMN = 8;

		float maxheight = size.toPixelsY(0.8f);
		float minheight = size.pixelSize.height() / MIN_ROWS_A_COLUMN;
		if (minheight < maxheight) {
			rowHeight = minheight;
		} else {
			rowHeight = maxheight;
		}

		for (int i = 0; i < COLUMN_COUNT; ++i) {
			Vector2F pos {rect.left + (columns[i].weight + columns[i].length / 2.0f) * (rect.width()), rect.top + rowHeight / 2.0f};
			columns[i].rect = Rectangle {
				rect.left + (columns[i].weight) * rect.width(), rect.top,
				rect.left + (columns[i].weight + columns[i].length) * rect.width(), rect.top + rowHeight
			};
		}

		updateScrollDetectorSize();
		scrollDetector.setWheelMultiplier(Vector2F {0.0f, rowHeight * 2.5f});

		if(selected >= 0) {
			animateIndex(selected);
		}
	}

	virtual void displayKeyboardSelection() override {
		selected = lastSelected;
	}
	virtual void hideKeyboardSelection() override {
		selected = -1;
	}

	virtual const char* getTitle() override {
		return "Levels";
	}

	void loadMoreLevels() {
		ASSERT(loadMoreVisible == LOADMORE_VISIBLE);
		loadMoreVisible = LOADMORE_LOADING;
		scene->getConnection().requestLevels();

		updateScrollDetectorSize();
	}

	void setSelectedIndex(unsigned int index) {
		if(index >= levelsCount) {
			if(loadMoreVisible != LOADMORE_INVISIBLE) {
				index = (int)levelsCount;
			} else {
				index = (int)levelsCount - 1;
			}
		}
		selected = lastSelected = index;
		animateIndex(index);
	}

	void selectLevel(unsigned int index) {
		ASSERT(index < levelsCount) << index;
		if(index >= levelsCount) {
			return;
		}

		const auto* desc = levels[index].descriptor;
		if(desc == nullptr) {
			desc = scene->getLevelWithUUID(levels[index].details->uuid);
		}
		DialogLayer* dialog;
		if(desc == nullptr) {
			dialog = new DownloadLevelDetailsLayer(this, levels[index]);
		} else {
			scene->levelQueriedOnServer(desc);
			Level level;
			if(!level.loadLevel(desc->getFileDescriptor())) {
				DialogLayer* info = new DialogLayer(parent);
				info->setTitle("Loading failed");
				info->addDialogItem(new TextDialogItem("Failed to load level. Please make sure you have the latest version of " SAPPHIRE_GAME_NAME "."));
				info->addDialogItem(new EmptyDialogItem(0.5f));
				info->addDialogItem(new CommandDialogItem( "Back", [=] {
									info->dismiss();
								}));
				dialog = info;
			} else {
				auto* ldl = new LevelDetailsLayer(parent, desc, util::move(level));
				dialog = ldl;
			}
		}
		dialog->show(scene, true);
	}

	void animateIndex(int index) {
		if (index < 0) {
			index = 0;
		}
		scrollDetector.animateTo(scrollDetector.getTargetScrollPosition(Rectangle {0, 0, rect.width(), rowHeight}.translate(Vector2F {0.0, rowHeight * (index)})));
	}
};
class DiscussionCommunityPage: public CommunityLayer::CommunityPage {
private:
	AutoResource<Font> font = getFont(ResIds::build::sipka_rh_font_convert::gameres::Consolas_ttf);

	rhfw::Resource<rhfw::render::Texture> editTextureBuffer;
	rhfw::Resource<rhfw::render::RenderTarget> editTextureRenderTarget;

	ScrollGestureDetector messagesScrollDetector;
	ScrollGestureDetector usersScrollDetector;
	float rowHeight = 0.0f;
	Rectangle rect;
	Rectangle messagesRect;
	Rectangle usersRect;

	char editTextContent[SAPPHIRE_DISCUSSION_MESSAGE_MAX_LEN + 1];
	unsigned int caretPosition = 0;
	unsigned int editTextLength = 0;

	Rectangle sendButtonRect;
	bool sendButtonDown = false;
	bool editRectDown = false;
	Rectangle editRect;

	float separatorWidth = 0.0f;

	core::time_millis lastCaretAction { 0 };

	bool caretVisible = false;

	bool softKeyboardEditing = false;

	CommunityConnection::DiscussionMessagesChangedListener::Listener discussionMessagesListener =
			CommunityConnection::DiscussionMessagesChangedListener::make_listener([=](unsigned int start, unsigned int count) {
				if(count == 0) {
					return;
				}
				CommunityConnection& conn = scene->getConnection();
				if(conn.getMessages().size() == 0) {
					if(count > 0) {
						unsigned int reqcount = min(count, (unsigned int)64);
						conn.requestMessages(start + count - reqcount, reqcount);
					}
				} else {
					unsigned int endindex = conn.getMessagesLocalStartIndex() + conn.getMessages().size();
					unsigned int reqcount = count - endindex;
					if(reqcount > 0) {
						conn.requestMessages(endindex, reqcount);
					}
				}
			});
	CommunityConnection::SendMessageListener::Listener sendMessageListener = CommunityConnection::SendMessageListener::make_listener(
			[=](uint32 userid, bool success) {
			});
	CommunityConnection::DiscussionMessageArrivedListener::Listener messageArrivedListener =
			CommunityConnection::DiscussionMessageArrivedListener::make_listener([=](unsigned int index) {
				updateMessageScrollDetector();
			});
	CommunityConnection::UserStateChangedListener::Listener userStateListener =
			CommunityConnection::UserStateChangedListener::make_listener([=](const CommunityConnection::OnlineUser& user, bool online) {
				updateUsersScrollDetector();
			});
public:
	DiscussionCommunityPage(SapphireScene* scene, CommunityLayer* parent)
			: CommunityPage(scene, parent) {
		editTextContent[0] = 0;

		auto&& conn = scene->getConnection();
		conn.discussionMessagesChangedEvents += discussionMessagesListener;
		conn.sendMessageEvents += sendMessageListener;
		conn.discussionMessageArrivedEvents += messageArrivedListener;
		conn.userStateChangedEvents += userStateListener;

		editTextureBuffer = renderer->createTexture();
		editTextureRenderTarget = renderer->createRenderTarget();
		render::RenderTargetDescriptor desc;
		desc.setColorTarget(editTextureBuffer);
		editTextureRenderTarget->setDescriptor(desc);

		messagesScrollDetector.setDragMultiplier(Vector2F { 1.0f, -1.0f });
	}
	~DiscussionCommunityPage() {
		editTextureBuffer.freeIfLoaded();
		editTextureRenderTarget.freeIfLoaded();
	}
	virtual void destroying() override {
		scene->getConnection().requestCommunityNotifications(false);
	}
	virtual void onCommunityConnected() override {
		auto&& conn = scene->getConnection();
		conn.requestCommunityNotifications(true);
		conn.queryMessageRanges();
	}
	virtual void onCommunityDisconnected() override {
		updateMessageScrollDetector();
		updateUsersScrollDetector();
	}
	virtual void draw(const Matrix2D& amvp, float alpha) override {
		if (caretVisible != shouldDisplayCaret()) {
			invalidateEditArea(!caretVisible);
		}
		float scroll = messagesScrollDetector.getPosition().y();
		float usersScroll = usersScrollDetector.getPosition().y();
		renderer->initDraw();

		const float rowtextsize = rowHeight * 0.9f;

		float usersleft = rect.right - rect.width() * 0.2f;
		float discussionwidth = usersleft - rect.left;

		FastFontDrawer& fontdrawer = parent->getFontDrawer();

		int userindex = 0;
		for (CommunityConnection::OnlineUser& c : scene->getConnection().getOnlineUsers().objects()) {
			Vector2F usertextpos { usersleft + rowtextsize / 3.0f, rect.top + rowHeight * userindex + usersScroll };
			if (usertextpos.y() > rect.bottom) {
				break;
			}
			const Color& color = Color { difficultyToColor(c.getDifficultyLevel()).xyz(), alpha };
			fontdrawer.add(c.getUserName(), color, usertextpos, rowtextsize, Gravity::LEFT | Gravity::TOP);
			++userindex;
		}
		drawRectangleColor(amvp, Color { parent->getUiColor().rgb(), alpha * 0.8f }, Rectangle { usersleft - separatorWidth, rect.top,
				usersleft, rect.bottom });

		ArrayList<SapphireDiscussionMessage>& msgs = scene->getConnection().getMessages();

		if (msgs.size() > 0) {
			Vector2F chattextpos { rect.left, rect.bottom - rowHeight * 1.5f - scroll };
			for (int i = msgs.size() - 1; chattextpos.y() > rect.top && i >= 0; --i) {
				auto* item = msgs.get(i);
				if (item == nullptr) {
					continue;
				}

				float userwidth;
				auto* usernamemeasure = font->measureText(item->getUserName(), rowtextsize, discussionwidth / 3.0f, &userwidth);
				const char* userappend;
				float width;
				if (*usernamemeasure != 0) {
					if (usernamemeasure == item->getUserName()) {
						++usernamemeasure;
					}
					userappend = "..: ";
					userwidth = font->measureText(item->getUserName(), usernamemeasure - 1, rowtextsize);
				} else {
					userappend = ": ";
				}
				userwidth += font->measureText(userappend, rowtextsize);

				unsigned int linecount = 1;
				float measurewidth = userwidth;
				for (auto* msg = item->getMessage(); true;) {
					auto* mend = font->measureText(msg, rowtextsize, discussionwidth - measurewidth);
					if (*mend == 0) {
						//remaining fits on line
						break;
					}
					if (mend == msg) {
						if (measurewidth == 0.0f) {
							//just in case of unexpected error, we advance by 1 char
							msg = msg + 1;
						}
						measurewidth = 0.0f;
						++linecount;
						continue;
					}
					auto* back = mend;
					while (back > msg && *back != ' ') {
						--back;
					}
					if (back == msg) {
						//found no space before
						//break the word in half, dont care, wont fit on the next line anyway
						measurewidth = 0.0f;
						++linecount;
						msg = mend;
						continue;
					}
					//here *back == ' '
					msg = back + 1;
					++linecount;
					measurewidth = 0.0f;
				}
				chattextpos.y() -= rowHeight * (linecount - 1);

				linecount = 0;

				float firstrowalpha = alpha;
				if (chattextpos.y() + linecount * rowHeight - rect.top < rowHeight) {
					firstrowalpha *= (chattextpos.y() + linecount * rowHeight - rect.top) / rowHeight;
				}
				if (chattextpos.y() > editRect.top) {
					float mult = 1 - (chattextpos.y() - editRect.top) / editRect.height();
					if (mult > 0) {
						firstrowalpha *= mult * mult;
					} else {
						firstrowalpha = 0.0f;
					}
				}
				if (firstrowalpha > 0.0f) {
					Color firstmsgcolor { difficultyToColor(item->getMessageDifficultyColor()).rgb(), firstrowalpha };
					if (*usernamemeasure != 0) {
						width = fontdrawer.add(item->getUserName(), usernamemeasure - 1, firstmsgcolor, chattextpos, rowtextsize,
								Gravity::LEFT | Gravity::BOTTOM);
					} else {
						width = fontdrawer.add(item->getUserName(), firstmsgcolor, chattextpos, rowtextsize,
								Gravity::LEFT | Gravity::BOTTOM);
					}
					width += fontdrawer.add(userappend, firstmsgcolor, Vector2F { chattextpos.x() + width, chattextpos.y() }, rowtextsize,
							Gravity::LEFT | Gravity::BOTTOM);
				}
				measurewidth = userwidth;
				for (auto* msg = item->getMessage(); true;) {
					float rowalpha = alpha;
					if (chattextpos.y() + linecount * rowHeight - rect.top < rowHeight) {
						rowalpha *= (chattextpos.y() + linecount * rowHeight - rect.top) / rowHeight;
					}
					if (chattextpos.y() + linecount * rowHeight > editRect.top) {
						float mult = 1 - (chattextpos.y() + linecount * rowHeight - editRect.top) / editRect.height();
						if (mult > 0) {
							rowalpha *= mult * mult;
						} else {
							rowalpha = 0.0f;
						}
					}
//							* (chattextpos.y() + linecount * rowHeight - rect.top < rowHeight ?
//									(chattextpos.y() + linecount * rowHeight - rect.top) / rowHeight : 1.0f);
					Color msgcolor { difficultyToColor(item->getMessageDifficultyColor()).rgb(), rowalpha };
					auto* mend = font->measureText(msg, rowtextsize, discussionwidth - measurewidth);
					if (*mend == 0) {
						//remaining fits on line
						if (rowalpha > 0.0f) {
							fontdrawer.add(msg, mend, msgcolor,
									Vector2F { chattextpos.x() + measurewidth, chattextpos.y() + linecount * rowHeight }, rowtextsize,
									Gravity::LEFT | Gravity::BOTTOM);
						}
						break;
					}
					if (mend == msg) {
						if (measurewidth == 0.0f) {
							//just in case of unexpected error, we advance by 1 char
							if (rowalpha > 0.0f) {
								fontdrawer.add(msg, msg + 1, msgcolor,
										Vector2F { chattextpos.x() + measurewidth, chattextpos.y() + linecount * rowHeight }, rowtextsize,
										Gravity::LEFT | Gravity::BOTTOM);
							}
							msg = msg + 1;
						}
						measurewidth = 0.0f;
						++linecount;
						continue;
					}
					auto* back = mend;
					while (back > msg && *back != ' ') {
						--back;
					}
					if (back == msg) {
						//found no space before
						//break the word in half, dont care, wont fit on the next line anyway
						if (rowalpha > 0.0f) {
							fontdrawer.add(msg, mend, msgcolor,
									Vector2F { chattextpos.x() + measurewidth, chattextpos.y() + linecount * rowHeight }, rowtextsize,
									Gravity::LEFT | Gravity::BOTTOM);
						}
						measurewidth = 0.0f;
						++linecount;
						msg = mend;
						continue;
					}
					//here *back == ' '
					if (rowalpha > 0.0f) {
						fontdrawer.add(msg, back + 1, msgcolor,
								Vector2F { chattextpos.x() + measurewidth, chattextpos.y() + linecount * rowHeight }, rowtextsize,
								Gravity::LEFT | Gravity::BOTTOM);
					}
					msg = back + 1;
					++linecount;
					measurewidth = 0.0f;
				}

				chattextpos.y() -= rowHeight;
			}
		} else {
			fontdrawer.add("No messages", Color { parent->getUiColor().rgb(), alpha },
					Vector2F { (rect.left + usersleft) / 2.0f, rect.top + (rect.height()) * 0.25f }, rowtextsize * 1.5, Gravity::CENTER);
		}

		if (sendButtonDown) {
			drawRectangleColor(amvp, Color { parent->getUiColor().rgb(), alpha }, sendButtonRect);
		}
		fontdrawer.add(SEND_MESSAGE_TEXT, Color { (sendButtonDown ? parent->getUiSelectedColor() : parent->getUiColor()).rgb(), alpha },
				sendButtonRect.middle(), rowtextsize, Gravity::CENTER);
	}
	virtual void postFontDraw(const Matrix2D& amvp, float alpha) override {
		drawSapphireTexture(amvp, editTextureBuffer, Color { 1, 1, 1, alpha }, editRect, Rectangle { 0, 0, 1, 1 });
	}
	void insertChar(char c) {
		if (caretPosition == editTextLength) {
			editTextContent[editTextLength++] = c;
			editTextContent[editTextLength] = 0;
		} else {
			memmove(editTextContent + caretPosition + 1, editTextContent + caretPosition,
					sizeof(char) * (editTextLength - caretPosition + 1));
			editTextContent[caretPosition] = c;
			++editTextLength;
		}
		++caretPosition;
	}
	void deleteChar() {
		if (caretPosition == editTextLength) {
			editTextContent[--editTextLength] = 0;
		} else {
			memmove(editTextContent + caretPosition - 1, editTextContent + caretPosition,
					sizeof(char) * (editTextLength - caretPosition + 1));
			--editTextLength;
		}
		--caretPosition;
	}
	void deleteForwardChar() {
		memmove(editTextContent + caretPosition, editTextContent + caretPosition + 1, sizeof(char) * (editTextLength - caretPosition));
		--editTextLength;
	}
	void clearEditTextContent() {
		editTextLength = 0;
		editTextContent[0] = 0;
		caretPosition = 0;
	}

	void setCaretAction() {
		lastCaretAction = core::MonotonicTime::getCurrent();
	}

	void sendMessage() {
		if (editTextLength > 0) {
			scene->getConnection().sendMessage(editTextContent, 0);
			clearEditTextContent();
			setCaretAction();
			invalidateEditArea(shouldDisplayCaret());
		}
	}

	bool isMessagesScrolling() const {
		return messagesScrollDetector.getWorkingSize().width() - messagesScrollDetector.getSize().width() > 0.5f
				|| messagesScrollDetector.getWorkingSize().height() - messagesScrollDetector.getSize().height() > 0.5f;
	}

	virtual bool onKeyEvent() override {
		if (KeyEvent::instance.getAction() == KeyAction::UNICODE_SEQUENCE) {
			unsigned int prevlen = editTextLength;
			unsigned int len = KeyEvent::instance.getUnicodeSequenceLength();
			const UnicodeCodePoint* seq = KeyEvent::instance.getUnicodeSequence();
			for (unsigned int i = 0; i < len && editTextLength < SAPPHIRE_DISCUSSION_MESSAGE_MAX_LEN; ++i) {
				UnicodeCodePoint cp = seq[i];
				if (cp >= 32 && cp <= 126) {
					insertChar((char) cp);
				}
			}
			if (prevlen != editTextLength) {
				setCaretAction();
				invalidateEditArea(shouldDisplayCaret());
			}
			return true;
		}
		if (KeyEvent::instance.getAction() == KeyAction::UNICODE_REPEAT) {
			const UnicodeCodePoint cp = KeyEvent::instance.getUnicodeRepeat();
			if (cp >= 32 && cp <= 126 && editTextLength < SAPPHIRE_DISCUSSION_MESSAGE_MAX_LEN) {
				unsigned int count = KeyEvent::instance.getUnicodeRepeatCount();
				for (unsigned int i = 0; i < count && editTextLength < SAPPHIRE_DISCUSSION_MESSAGE_MAX_LEN; ++i) {
					insertChar((char) cp);
				}
				setCaretAction();
				invalidateEditArea(shouldDisplayCaret());
			}
			return true;
		}
		switch (KeyEvent::instance.getKeycode()) {
			case KeyCode::KEY_GAMEPAD_DPAD_LEFT:
			case KeyCode::KEY_DIR_LEFT: {
				BREAK_ON_NOT_DOWN();
				setCaretAction();
				if(caretPosition > 0) {
					--caretPosition;
					invalidateEditArea(shouldDisplayCaret());
				}
				break;
			}
			case KeyCode::KEY_GAMEPAD_DPAD_RIGHT:
			case KeyCode::KEY_DIR_RIGHT: {
				BREAK_ON_NOT_DOWN();
				setCaretAction();
				if(caretPosition < editTextLength) {
					++caretPosition;
					invalidateEditArea(shouldDisplayCaret());
				}
				break;
			}
			case KeyCode::KEY_GAMEPAD_DPAD_UP:
			case KeyCode::KEY_DIR_UP: {
				BREAK_ON_NOT_DOWN();
				if(isMessagesScrolling()) {
					messagesScrollDetector.applyWheel(1.0f);
				}
				break;
			}
			case KeyCode::KEY_GAMEPAD_DPAD_DOWN:
			case KeyCode::KEY_DIR_DOWN: {
				BREAK_ON_NOT_DOWN();
				if(isMessagesScrolling()) {
					messagesScrollDetector.applyWheel(-1.0f);
				}
				break;
			}
			case KeyCode::KEY_GAMEPAD_Y:
			case KeyCode::KEY_BACKSPACE: {
				BREAK_ON_NOT_DOWN();
				setCaretAction();
				if(editTextLength > 0 && caretPosition > 0) {
					deleteChar();
					invalidateEditArea(shouldDisplayCaret());
				}
				break;
			}
			case KeyCode::KEY_GAMEPAD_X:
			case KeyCode::KEY_DELETE: {
				BREAK_ON_NOT_DOWN();
				setCaretAction();
				if(editTextLength > 0 && caretPosition < editTextLength) {
					deleteForwardChar();
					invalidateEditArea(shouldDisplayCaret());
				}
				break;
			}
			case KeyCode::KEY_HOME: {
				BREAK_ON_NOT_DOWN();
				if ((KeyEvent::instance.getModifiers() & KeyModifiers::CTRL_ON_BOOL_MASK) != 0) {
					messagesScrollDetector.animateTo(messagesScrollDetector.getScrollRange().rightBottom());
				} else {
					setCaretAction();
					if(caretPosition != 0) {
						caretPosition = 0;
						invalidateEditArea(shouldDisplayCaret());
					}
				}
				break;
			}
			case KeyCode::KEY_END: {
				BREAK_ON_NOT_DOWN();
				if ((KeyEvent::instance.getModifiers() & KeyModifiers::CTRL_ON_BOOL_MASK) != 0) {
					messagesScrollDetector.animateTo(messagesScrollDetector.getScrollRange().leftTop());
				} else {
					setCaretAction();
					if(caretPosition != editTextLength) {
						caretPosition = editTextLength;
						invalidateEditArea(shouldDisplayCaret());
					}
				}
				break;
			}
			case KeyCode::KEY_PAGE_DOWN: {
				BREAK_ON_NOT_DOWN();
				messagesScrollDetector.animateTo(messagesScrollDetector.getPosition() + messagesScrollDetector.getSize());
				break;
			}
			case KeyCode::KEY_PAGE_UP: {
				BREAK_ON_NOT_DOWN();
				messagesScrollDetector.animateTo(messagesScrollDetector.getPosition() - messagesScrollDetector.getSize());
				break;
			}
			case KeyCode::KEY_GAMEPAD_A:
			case KeyCode::KEY_ENTER: {
				BREAK_ON_NOT_DOWN();
				sendMessage();
				break;
			}
			case KeyCode::KEY_GAMEPAD_B:
			case KeyCode::KEY_GAMEPAD_BACK:
			case KeyCode::KEY_BACK: {
				if(softKeyboardEditing) {
					BREAK_ON_NOT_DOWN();
					softKeyboardEditing = false;
					scene->getWindow()->dismissSoftKeyboard();
				} else {
					return false;
				}
				break;
			}
			default: {
				if(KeyEvent::instance.getInputDevice() == InputDevice::KEYBOARD) {
					if (scene->getWindow()->isSoftKeyboardShowing()) {
						softKeyboardEditing = false;
						scene->getWindow()->dismissSoftKeyboard();
						return true;
					}
				}
				return false;
			}
		}
		return true;
	}
	virtual void touch(TouchAction action, const Vector2F& touchpos) override {
		//TODO check if isinside
		messagesScrollDetector.onTouch(!sendButtonDown && !editRectDown && messagesRect.isInside(touchpos));
		usersScrollDetector.onTouch(!sendButtonDown && !editRectDown && usersRect.isInside(touchpos));
		switch (action) {
			case TouchAction::CANCEL: {
				sendButtonDown = false;
				editRectDown = false;
				break;
			}
			case TouchAction::DOWN: {
				if (sendButtonRect.isInside(touchpos)) {
					sendButtonDown = true;
				} else if (editRect.isInside(touchpos)) {
					editRectDown = true;
				}
				break;
			}
			case TouchAction::MOVE_UPDATE: {
				if (sendButtonDown) {
					if (!sendButtonRect.isInside(touchpos)) {
						sendButtonDown = false;
					}
				} else if (editRectDown) {
					if (!editRect.isInside(touchpos)) {
						editRectDown = false;
					}
				}
				break;
			}
			case TouchAction::UP: {
				if (sendButtonDown) {
					sendButtonDown = false;
					sendMessage();
				} else if (editRectDown) {
					editRectDown = false;
					if (softKeyboardEditing) {
						scene->getWindow()->dismissSoftKeyboard();
						softKeyboardEditing = false;
					} else {
						if (!scene->getWindow()->isHardwareKeyboardPresent()) {
							scene->getWindow()->requestSoftKeyboard(KeyboardType::ALPHANUMERIC);
							softKeyboardEditing = true;
						}
					}
				} else if (softKeyboardEditing) {
					if (softKeyboardEditing) {
						scene->getWindow()->dismissSoftKeyboard();
						softKeyboardEditing = false;
					}
				}
				break;
			}
			default: {
				break;
			}
		}
	}
	virtual void onDismissingPage() override {
		if (softKeyboardEditing) {
			scene->getWindow()->dismissSoftKeyboard();
			softKeyboardEditing = false;
		}
	}

	unsigned int countLines() {
		unsigned int linecount = 0;
		ArrayList<SapphireDiscussionMessage>& msgs = scene->getConnection().getMessages();
		const float rowtextsize = rowHeight * 0.9f;
		float usersleft = rect.right - rect.width() * 0.2f;
		float discussionwidth = usersleft - rect.left;
		for (int i = 0; i < msgs.size(); ++i) {
			auto* item = msgs.get(i);
			if (item == nullptr) {
				continue;
			}

			float userwidth;
			auto* usernamemeasure = font->measureText(item->getUserName(), rowtextsize, discussionwidth / 3.0f, &userwidth);
			const char* userappend;
			float width;
			if (*usernamemeasure != 0) {
				if (usernamemeasure == item->getUserName()) {
					++usernamemeasure;
				}
				userappend = "..: ";
				userwidth = font->measureText(item->getUserName(), usernamemeasure - 1, rowtextsize);
			} else {
				userappend = ": ";
			}
			userwidth += font->measureText(userappend, rowtextsize);

			++linecount;
			float measurewidth = userwidth;
			for (auto* msg = item->getMessage(); true;) {
				auto* mend = font->measureText(msg, rowtextsize, discussionwidth - measurewidth);
				if (*mend == 0) {
					//remaining fits on line
					break;
				}
				if (mend == msg) {
					if (measurewidth == 0.0f) {
						//just in case of unexpected error, we advance by 1 char
						msg = msg + 1;
					}
					measurewidth = 0.0f;
					++linecount;
					continue;
				}
				auto* back = mend;
				while (back > msg && *back != ' ') {
					--back;
				}
				if (back == msg) {
					//found no space before
					//break the word in half, dont care, wont fit on the next line anyway
					measurewidth = 0.0f;
					++linecount;
					msg = mend;
					continue;
				}
				//here *back == ' '
				msg = back + 1;
				++linecount;
				measurewidth = 0.0f;
			}
		}
		return linecount;
	}
	unsigned int countUsers() {
		unsigned int result = 0;
		for (auto&& c : scene->getConnection().getOnlineUsers().objects()) {
			++result;
		}
		return result;
	}

	void updateMessageScrollDetector() {
		int linecount = countLines();
		messagesScrollDetector.setSize(Vector2F { rect.width(), rect.height() - rowHeight * 1.5f },
				Vector2F { rect.width(), linecount * rowHeight });
	}
	void updateUsersScrollDetector() {
		int usercount = countUsers();
		usersScrollDetector.setSize(Vector2F { rect.width(), rect.height() }, Vector2F { rect.width(), usercount * rowHeight });
	}

	virtual void sizeChanged(const rhfw::core::WindowSize& size) override {
		float padding = min(size.toPixelsX(0.5f), size.pixelSize.width() / 12.0f);
		this->rect = {padding, padding / 3.0f, size.pixelSize.width() - padding, (float) size.pixelSize.height()};

		const int MIN_ROWS_A_COLUMN = 8;

		float maxheight = size.toPixelsY(0.8f);
		float minheight = size.pixelSize.height() / MIN_ROWS_A_COLUMN;
		if (minheight < maxheight) {
			rowHeight = minheight;
		} else {
			rowHeight = maxheight;
		}
		separatorWidth = size.toPixelsX(0.08f);

		float sendwidth = font->measureText(SEND_MESSAGE_TEXT, rowHeight * 0.9f);
		float usersleft = rect.right - rect.width() * 0.2f;
		sendButtonRect = Rectangle { usersleft - separatorWidth - sendwidth * 1.6f, rect.bottom - rowHeight * 1.5f, usersleft
				- separatorWidth, rect.bottom - rowHeight * 0.5f };

		editRect = Rectangle { rect.left, rect.bottom - rowHeight * 1.5f, sendButtonRect.left, rect.bottom - rowHeight * 0.5f };

		editTextureBuffer->setInputSource(new render::EmptyInputSource { Size2UI { editRect.widthHeight() }, ColorFormat::RGBA_8888 });
		editTextureBuffer.loadOrReload();
		editTextureRenderTarget.loadOrReload();

		this->messagesRect = Rectangle { rect.left, rect.top, sendButtonRect.right, rect.bottom };
		this->usersRect = Rectangle { this->messagesRect.right, rect.top, rect.right, rect.bottom };

		updateMessageScrollDetector();
		updateUsersScrollDetector();
		messagesScrollDetector.setWheelMultiplier(Vector2F { 0.0f, rowHeight * -3.0f });
		usersScrollDetector.setWheelMultiplier(Vector2F { 0.0f, rowHeight * 3.0f });

		invalidateEditArea(shouldDisplayCaret());
	}

	bool shouldDisplayCaret() {
		return (((long long) ((core::time_millis) core::MonotonicTime::getCurrent() - lastCaretAction)) / 1000) % 2 == 0;
	}

	void invalidateEditArea(bool caret) {
		renderer->pushRenderTarget(editTextureRenderTarget);
		{
			auto vp = renderer->getViewPort();
			renderer->resetViewPort();
			renderer->initDraw();

			auto bufsize = editTextureBuffer->getSize();
			renderer->clearColor(parent->getUiColor());
			auto mvp = Matrix2D { }.setScreenDimension(bufsize).multRenderToTexture(renderer);
			auto inrect = Rectangle { 0, 0, (float) bufsize.width(), (float) bufsize.height() }.inset(rowHeight * 0.05f);
			drawRectangleColor(mvp, Color { 0, 0, 0, 1.0f }, inrect);

			float textsize = rowHeight * 0.9f;

			float caretstart = font->measureText(editTextContent, editTextContent + caretPosition, textsize);

			Vector2F textpos { rowHeight * 0.1f, editRect.height() / 2.0f };
			if (caretstart > inrect.width() - textpos.x()) {
				float diff = caretstart - (inrect.width() - textpos.x());
				mvp = (Matrix2D { }.setTranslate(-diff, 0) *= mvp);
			}

			drawString(mvp, editTextContent, font, parent->getUiColor(), textpos, textsize, Gravity::CENTER_VERTICAL | Gravity::LEFT);

			if (caret) {
				drawRectangleColor(mvp, Color { 1, 1, 1, 0.8f },
						Rectangle { textpos.x() + caretstart, inrect.top, textpos.x() + caretstart + textsize / 8.0f, inrect.bottom });
			}
			caretVisible = caret;

			renderer->setViewPort(vp);
		}
		renderer->popRenderTarget();
	}

	virtual void displayKeyboardSelection() override {
	}

	virtual const char* getTitle() override {
		return "Discussion";
	}
};

CommunityLayer::CommunityLayer(SapphireUILayer* parent)
		: SapphireUILayer(parent) {
	helloStringBuffer[0] = 0;

	this->returnText = "Return to community";
}
CommunityLayer::~CommunityLayer() {
	for (int i = 0; i < PAGE_COUNT; ++i) {
		delete pages[i];
	}
}
float CommunityLayer::adjustSoftKeyboardTranslation(float y) {
	if (getScene()->getWindow()->isSoftKeyboardShowing()) {
		return y + (static_cast<SapphireScene*>(getScene())->getUiSize().pixelSize.height() * 0.6f);
	}
	return y;
}
void CommunityLayer::drawImpl(float displaypercent) {
	SapphireScene* ss = static_cast<SapphireScene*>(getScene());
	auto size = ss->getUiSize();
	renderer->setDepthTest(false);
	renderer->initDraw();

	float alpha = displaypercent * displaypercent;

	auto mvp = Matrix2D { }.setTranslate(0, -adjustSoftKeyboardTranslation(0.0f)).multScreenDimension(size.pixelSize);

	fontDrawerPool.prepare(font, mvp);
	fontDrawer.prepare(4096);

	if (backTouch != nullptr) {
		drawRectangleColor(mvp, Color { getUiColor().rgb(), alpha }, backButtonPos);
	}
	if (helloTouch != nullptr) {
		drawRectangleColor(mvp, Color { getUiColor().rgb(), alpha }, helloRectangle);
	}
	drawSapphireTexture(mvp, backIconWhite, backTouch != nullptr ? Color { getUiSelectedColor().rgb(), alpha } : Color { getUiColor().rgb(),
																			alpha }, backButtonPos, Rectangle { 0, 0, 1, 1 });

	if (ss->getConnection().isTaskRunning()) {
		float calpha = alpha;
		if (!ss->getConnection().isConnectionEstablished()) {
			calpha = calpha * (sinf(((long long) core::time_millis { core::MonotonicTime::getCurrent() } % 1000) / 1000.0f * M_PI * 2) + 1)
					/ 2.0f;
		} else {
			long long msdiff = ((long long) (core::time_millis { core::MonotonicTime::getCurrent() } - connectedTime));
			if (msdiff >= 8000) {
				goto label_after_network_icon_draw;
			}
			calpha = calpha * ((8000 - msdiff) / 8000.0f);
		}
		drawSapphireTexture(mvp, networkConnectIconWhite, Color { getUiColor().rgb(), calpha }, connectionRectangle,
				Rectangle { 0, 0, 1, 1 });
	} else {
		if (connectionTouch != nullptr) {
			drawRectangleColor(mvp, Color { getUiColor().rgb(), alpha }, connectionRectangle);
		}
		drawSapphireTexture(mvp, networkDisconnectIconWhite, Color {
				(connectionTouch != nullptr ? getUiSelectedColor() : getUiColor()).rgb(), alpha }, connectionRectangle, Rectangle { 0, 0, 1,
				1 });
	}

	label_after_network_icon_draw:

	fontDrawer.add(TITLE_TEXT, Color { getUiColor().rgb(), alpha }, Vector2F { backButtonPos.right, backButtonPos.middle().y() },
			titleTextSize, Gravity::CENTER_VERTICAL | Gravity::LEFT);

	fontDrawer.add(helloStringBuffer, Color { getSelectedUiColor(helloTouch != nullptr).rgb(), alpha }, helloRectangle.middle(),
			titleTextSize / 2.5f, Gravity::CENTER);

#if RHFW_DEBUG
	{
		char buf[64];
		sprintf(buf, "UserScore: %u", ss->getUserProgressScore());
		fontDrawer.add(buf, Color { getUiColor().rgb(), alpha }, Vector2F { 0, 0 }, titleTextSize / 2.5f, Gravity::LEFT | Gravity::TOP);
	}
#endif /* RHFW_DEBUG */

	for (unsigned int i = 0; i < PAGE_COUNT; ++i) {
		bool highlight = currentPage == i || pageTitleTouch[i] != nullptr;
		if (highlight) {
			drawRectangleColor(mvp, Color { getUiColor().rgb(), alpha }, pageTitleDisplayRectangles[i]);
		}
		float width = fontDrawer.add(pages[i]->getTitle(), Color { getSelectedUiColor(highlight).rgb(), alpha },
				pageTitleRectangles[i].middle(), titleTextSize / 2.5f, Gravity::CENTER);
	}

	auto pagemvp = (Matrix2D { }.setTranslate(0.0f, titleTextSize * 1.5f) *= mvp);

	fontDrawerPool.commit();
	fontDrawerPool.prepare(font, pagemvp);
	fontDrawer.prepare(4096);
	for (unsigned int i = 0; i < PAGE_COUNT; ++i) {
		float pagealpha = pageAlphas[i] * pageAlphas[i] * alpha;
		if (pagealpha > 0.0f) {
			pages[i]->draw(pagemvp, pagealpha);
		}
	}
	fontDrawerPool.commit();
	for (unsigned int i = 0; i < PAGE_COUNT; ++i) {
		float pagealpha = pageAlphas[i] * pageAlphas[i] * alpha;
		if (pagealpha > 0.0f) {
			pages[i]->postFontDraw(pagemvp, pagealpha);
		}
	}
}

void CommunityLayer::setCurrentPage(unsigned int index) {
	ASSERT(index < PAGE_COUNT);
	if (index == currentPage) {
		return;
	}

	pages[currentPage]->onDismissingPage();
	core::time_millis duration { 250 };
	auto start = core::MonotonicTime::getCurrent();
	auto fade = new PropertyAnimator<float>(pageAlphas[currentPage], 0.0f, start, duration);
	pageAnimations[currentPage].kill();
	pageAnimations[currentPage].link(fade);
	currentPage = index;
	auto display = new PropertyAnimator<float>(pageAlphas[currentPage], 1.0f, start, duration);
	pageAnimations[currentPage].kill();
	pageAnimations[currentPage].link(display);

	pages[currentPage]->onDisplayingPage();

	fade->start();
	display->start();
}

bool CommunityLayer::onKeyEventImpl() {
	if (pages[currentPage]->onKeyEvent()) {
		return true;
	}
	switch (KeyEvent::instance.getKeycode()) {
		case KeyCode::KEY_GAMEPAD_RIGHT_SHOULDER:
		case KeyCode::KEY_TAB: {
			BREAK_ON_NOT_DOWN();
			setCurrentPage((currentPage + 1) % PAGE_COUNT);
			break;
		}
		case KeyCode::KEY_GAMEPAD_LEFT_SHOULDER: {
			BREAK_ON_NOT_DOWN();
			unsigned int cp = currentPage - 1;
			if(cp >= PAGE_COUNT) {
				cp = PAGE_COUNT - 1;
			}
			setCurrentPage(cp);
			break;
		}
		case KeyCode::KEY_GAMEPAD_START:
		case KeyCode::KEY_F5: {
			BREAK_ON_NOT_DOWN();
			if(!static_cast<SapphireScene*>(getScene())->getConnection().isTaskRunning()) {
				static_cast<SapphireScene*>(getScene())->getConnection().connect(static_cast<SapphireScene*>(getScene()));
			}
			break;
		}
		case KeyCode::KEY_F1: {
			BREAK_ON_NOT_DOWN();
			showUserDetailsEditDialog();
			break;
		}
		default: {
			return SapphireUILayer::onKeyEventImpl();
		}
	}
	return true;
}
bool CommunityLayer::touchImpl() {
	if (!showing || !gainedInput || TouchEvent::instance.getPointerCount() > 1 || TouchEvent::instance.getAction() == TouchAction::CANCEL) {
		if (pageTouch != nullptr) {
			pageTouch->touch(TouchAction::CANCEL, Vector2F { 0, 0 });
			pageTouch = nullptr;
			pageTouchPointer = nullptr;
		}
		backTouch = nullptr;
		helloTouch = nullptr;
		connectionTouch = nullptr;
		for (unsigned int i = 0; i < PAGE_COUNT; ++i) {
			pageTitleTouch[i] = nullptr;
		}
		return true;
	}
	auto* affected = TouchEvent::instance.getAffectedPointer();
	if (affected == nullptr) {
		return false;
	}
	auto touchpos = affected->getPosition();
	touchpos.y() = adjustSoftKeyboardTranslation(touchpos.y());

	switch (TouchEvent::instance.getAction()) {
		case TouchAction::DOWN: {
			pageTouch = nullptr;
			if (backButtonPos.isInside(touchpos)) {
				backTouch = affected;
			} else if (helloRectangle.isInside(touchpos)) {
				helloTouch = affected;
			} else if (connectionRectangle.isInside(touchpos)) {
				connectionTouch = affected;
			} else {
				for (unsigned int i = 0; i < PAGE_COUNT; ++i) {
					if (pageTitleRectangles[i].isInside(touchpos)) {
						pageTitleTouch[i] = affected;
						goto label_skip_pagecheck;
					}
				}
				pageTouch = pages[currentPage];
				pageTouch->touch(TouchAction::DOWN, Vector2F { touchpos.x(), touchpos.y() - helloRectangle.bottom });
				pageTouchPointer = affected;
				label_skip_pagecheck: ;
			}
			break;
		}
		case TouchAction::MOVE_UPDATE: {
			if (pageTouchPointer == affected) {
				pageTouch->touch(TouchAction::MOVE_UPDATE, Vector2F { touchpos.x(), touchpos.y() - helloRectangle.bottom });
			}
			if (backTouch == affected && !backButtonPos.isInside(touchpos)) {
				backTouch = nullptr;
			}
			if (helloTouch == affected && !helloRectangle.isInside(touchpos)) {
				helloTouch = nullptr;
			}
			if (connectionTouch == affected && !connectionRectangle.isInside(touchpos)) {
				connectionTouch = nullptr;
			}
			for (unsigned int i = 0; i < PAGE_COUNT; ++i) {
				if (pageTitleTouch[i] == affected && !pageTitleRectangles[i].isInside(touchpos)) {
					pageTitleTouch[i] = nullptr;
				}
			}
			break;
		}
		case TouchAction::UP: {
			if (pageTouchPointer == affected) {
				pageTouch->touch(TouchAction::UP, Vector2F { touchpos.x(), touchpos.y() - helloRectangle.bottom });
				pageTouch = nullptr;
				pageTouchPointer = nullptr;
			} else if (backTouch == affected) {
				backTouch = nullptr;
				dismiss();
			} else if (helloTouch == affected) {
				showUserDetailsEditDialog();
				helloTouch = nullptr;
			} else if (connectionTouch == affected) {
				if (!static_cast<SapphireScene*>(getScene())->getConnection().isTaskRunning()) {
					static_cast<SapphireScene*>(getScene())->getConnection().connect(static_cast<SapphireScene*>(getScene()));
				}
				connectionTouch = nullptr;
			} else {
				for (unsigned int i = 0; i < PAGE_COUNT; ++i) {
					if (pageTitleTouch[i] == affected) {
						setCurrentPage(i);
						pageTitleTouch[i] = nullptr;
						break;
					}
				}
			}
			break;
		}
		case TouchAction::WHEEL: {
			pages[currentPage]->touch(TouchAction::WHEEL, Vector2F { touchpos.x(), touchpos.y() - helloRectangle.bottom });
			break;
		}
		case TouchAction::SCROLL: {
			pages[currentPage]->touch(TouchAction::WHEEL, Vector2F { touchpos.x(), touchpos.y() - helloRectangle.bottom });
			break;
		}
		default: {
			break;
		}
	}
	return true;
}

void CommunityLayer::sizeChanged(const rhfw::core::WindowSize& size) {
	SapphireUILayer::sizeChanged(size);

	float titleheight = size.pixelSize.height() * SAPPHIRE_TITLE_PERCENT;
	if (titleheight > size.toPixelsY(3.5f)) {
		titleheight = size.toPixelsY(3.5f);
	}
	float titlew = titleheight * 2 + font->measureText(TITLE_TEXT, titleheight);
	if (titlew > size.pixelSize.width() * 7.0f / 8.0f) {
		titleheight = titleheight / titlew * size.pixelSize.width() * 7.0f / 8.0f;
		titlew = size.pixelSize.width() * 7.0f / 8.0f;
	}
	titleTextSize = titleheight;
	backButtonPos = Rectangle { (size.pixelSize.width() - titlew) / 2.0f, 0, (size.pixelSize.width() - titlew) / 2.0f + titleheight,
			titleheight };
	connectionRectangle = Rectangle { size.pixelSize.width() - backButtonPos.width() * 5 / 6, backButtonPos.height() * 1 / 6,
			size.pixelSize.width() - backButtonPos.width() * 1 / 6, backButtonPos.height() * 5 / 6 };

	fillHelloStringBuffer();

	relayout(size);
}

void CommunityLayer::relayout(const core::WindowSize& size) {
	helloRectangle.left = 0;
	helloRectangle.top = titleTextSize;
	helloRectangle.bottom = helloRectangle.top + titleTextSize / 2.0f;
	helloRectangle.right = font->measureText((const char*) helloStringBuffer, titleTextSize / 2.5f) + titleTextSize / 2.0f;
	if (helloRectangle.right > size.pixelSize.width() / 2.0f) {
		helloRectangle.right = size.pixelSize.width() / 2.0f;
		char* last = font->measureText((char*) helloStringBuffer, titleTextSize / 2.5f, helloRectangle.right);
		auto count = last - helloStringBuffer;
		*last-- = 0;
		for (int i = 0; i < 2 && last >= helloStringBuffer; ++i) {
			*last-- = '.';
		}
	}

	for (unsigned int i = 0; i < PAGE_COUNT; ++i) {
		pageTitleRectangles[i].left = helloRectangle.right + (size.pixelSize.width() - helloRectangle.right) / PAGE_COUNT * i;
		pageTitleRectangles[i].right = pageTitleRectangles[i].left + (size.pixelSize.width() - helloRectangle.right) / PAGE_COUNT;
		pageTitleRectangles[i].top = helloRectangle.top;
		pageTitleRectangles[i].bottom = helloRectangle.bottom;

		float titlewidth = font->measureText(pages[i]->getTitle(), titleTextSize / 2.5f);

		if (titlewidth * 2.0f >= pageTitleRectangles[i].width()) {
			pageTitleDisplayRectangles[i] = pageTitleRectangles[i];
		} else {
			pageTitleDisplayRectangles[i] = pageTitleRectangles[i].inset(
					Vector2F { (pageTitleRectangles[i].width() - titlewidth * 2.0f) / 2.0f, 0 });
		}

		core::WindowSize pagesize { size };
		pagesize.pixelSize.height() -= titleTextSize + helloRectangle.height();
		pages[i]->sizeChanged(pagesize);
	}
}

void CommunityLayer::setScene(Scene* scene) {
	SapphireUILayer::setScene(scene);
	SapphireScene* ss = static_cast<SapphireScene*>(scene);
	if (pages[0] == nullptr) {
		pages[0] = new LevelsCommunityPage(ss, this);
		pages[1] = new DiscussionCommunityPage(ss, this);
	}
	pages[currentPage]->onDisplayingPage();
	ss->settingsChangedListeners += *this;
	ss->levelStateListeners += levelStateListener;

	this->uiDifficultyColor = ss->getUserDifficultyColor();
	setColors(uiDifficultyColor);

	ss->getConnection().communityInformationEvents += informationListener;
	ss->getConnection().addStateListenerAndConnect(ss, *this);

}
void CommunityLayer::onSettingsChanged(const SapphireSettings& settings) {
	fillHelloStringBuffer();
	relayout(static_cast<SapphireScene*>(getScene())->getUiSize());
}

void CommunityLayer::fillHelloStringBuffer() {
	const char* name = (const char*) static_cast<SapphireScene*>(getScene())->getCurrentUserName();
	if (name == nullptr || *name == 0) {
#define NAME_NULLPTR_PLACEHOLDER "Hello " SAPPHIRE_PLAYER_PLACEHOLDER_NAME "!"
		memcpy(helloStringBuffer, NAME_NULLPTR_PLACEHOLDER, sizeof(NAME_NULLPTR_PLACEHOLDER));
	} else {
		sprintf(helloStringBuffer, "Hello %s!", name);
	}

}

void CommunityLayer::onConnected(CommunityConnection* connection) {
	connectedTime = core::MonotonicTime::getCurrent();
}
void CommunityLayer::onLoggedIn(CommunityConnection* connection) {
	SapphireScene* ss = static_cast<SapphireScene*>(getScene());
	if (ss->getCurrentUserName().length() > 0) {
		connection->updatePlayerData(ss->getCurrentUserName(), ss->getUserDifficultyColor());
	}
	for (unsigned int i = 0; i < PAGE_COUNT; ++i) {
		pages[i]->onCommunityConnected();
	}
}

void CommunityLayer::onDisconnected(CommunityConnection* connection) {
	for (unsigned int i = 0; i < PAGE_COUNT; ++i) {
		pages[i]->onCommunityDisconnected();
	}
}

void CommunityLayer::onConnectionFailed(CommunityConnection* connection) {
}

void CommunityLayer::showUserDetailsEditDialog() {
	auto* ss = static_cast<SapphireScene*>(getScene());

	auto* dialog = new DialogLayer(this);
	dialog->setTitle("User details");
	auto* nickitem = new EditTextDialogItem("Nickname:", ss->getCurrentUserName());
	nickitem->setContentMaximumLength(SAPPHIRE_USERNAME_MAX_LEN);
	dialog->addDialogItem(nickitem);
	dialog->addDialogItem(new EmptyDialogItem(0.5f));
	auto* acceptitem = new CommandDialogItem("Accept", [=] {
		if(nickitem->getContentLength() < SAPPHIRE_USERNAME_MIN_LEN) {
			showShortNicknameInfoDialog(dialog);
		} else {
			ss->setCurrentUserName(nickitem->getContent());
			ss->getConnection().updatePlayerData(ss->getCurrentUserName(), uiDifficultyColor);
			fillHelloStringBuffer();
			dialog->dismiss();
		}
	});
	dialog->addDialogItem(acceptitem);
	nickitem->setReturnHandler([=] {
		acceptitem->onSelected(nullptr);
		return true;
	});
	dialog->addDialogItem(new CommandDialogItem("Cancel", [=] {
		dialog->dismiss();
	}));

	dialog->show(getScene(), true);
}

void CommunityLayer::showNickNameRequestDialog() {
	auto* ss = static_cast<SapphireScene*>(getScene());
	ParentDismisserDialogLayer* dialog = new ParentDismisserDialogLayer(this);

	auto* nicknameitem = new EditTextDialogItem("Nickname:", nullptr);
	nicknameitem->setContentMaximumLength(SAPPHIRE_USERNAME_MAX_LEN);
	dialog->setTitle("Player name");
	dialog->addDialogItem(new TextDialogItem("Enter your nickname!"));
	dialog->addDialogItem(nicknameitem);
	dialog->addDialogItem(new EmptyDialogItem(0.5f));
	auto* acceptitem = new CommandDialogItem("Accept", [=] {
		if(nicknameitem->getContentLength() < SAPPHIRE_USERNAME_MIN_LEN) {
			showShortNicknameInfoDialog(dialog);
		} else {
			ss->setCurrentUserName(nicknameitem->getContentString());
			ss->getConnection().updatePlayerData(ss->getCurrentUserName(), ss->getUserDifficultyColor());
			dialog->dismissKeepParent();
		}
	});
	dialog->addDialogItem(acceptitem);
	nicknameitem->setReturnHandler([=] {
		acceptitem->onSelected(nullptr);
		return true;
	});
	dialog->addDialogItem(new CommandDialogItem("Cancel", [=] {
		dialog->dismiss();
	}));
	dialog->show(ss, true);
}

void CommunityLayer::showShortNicknameInfoDialog(SapphireUILayer* parent) {
	auto* info = new DialogLayer(parent);
	info->setTitle("Invalid nickname");
	info->addDialogItem(new TextDialogItem("Your nickname must be at last " STRINGIZE(SAPPHIRE_USERNAME_MIN_LEN) " character long!"));
	info->addDialogItem(new EmptyDialogItem(0.5f));
	info->addDialogItem(new CommandDialogItem("Ok", [=] {
		info->dismiss();
	}));
	info->show(getScene(), true);
}

void CommunityLayer::dismiss() {
	SapphireUILayer::dismiss();
	for (unsigned int i = 0; i < PAGE_COUNT; ++i) {
		if (i == currentPage) {
			pages[i]->onDismissingPage();
		}
		pages[i]->destroying();
	}
}

void CommunityLayer::showInitDialogs() {
	SapphireScene* ss = static_cast<SapphireScene*>(getScene());
	if (ss->getCurrentUserName().length() == 0) {
		showNickNameRequestDialog();
	}
}

} // namespace userapp

